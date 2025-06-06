#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <cmath>
#include <winsock2.h>
#include <windows.h>
#include <setupapi.h>
#include <initguid.h>
#include <usbiodef.h>
#include <cstdint>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "setupapi.lib")

constexpr bool DEBUG = true;

#define DEBUG_TIMESTAMP(label) \
    if constexpr (DEBUG) { \
        static auto last = std::chrono::high_resolution_clock::now(); \
        auto now = std::chrono::high_resolution_clock::now(); \
        std::cout << label << ": " \
                  << std::chrono::duration_cast<std::chrono::microseconds>(now - last).count() \
                  << " us" << std::endl; \
        last = now; \
    }

uint8_t crc8_table[256];
uint16_t crc16_table[256];

void generate_crc8_table() {
    const uint8_t polynomial = 0x37; //odrive docs
    for (int i = 0; i < 256; ++i) {
        uint8_t crc = static_cast<uint8_t>(i);
        for (int bit = 0; bit < 8; ++bit) {
            if ((crc & 0x80) != 0) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
        crc8_table[i] = crc;
    }
}

void generate_crc16_table() {
    const uint16_t polynomial = 0x3d65; //odrive docs

    for (int i = 0; i < 256; ++i) {
        uint16_t crc = static_cast<uint16_t>(i << 8); // align byte to high bits for no reflection (odrive docs)
        for (int bit = 0; bit < 8; ++bit) {
            if ((crc & 0x8000) != 0) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
        crc16_table[i] = crc;
    }
}

uint8_t compute_crc8(const uint8_t* data, size_t length) {
    uint8_t crc = 0x42; // odrive docs
    for (size_t i = 0; i < length; ++i) {
        crc = crc8_table[crc ^ data[i]];
    }
    return crc;
}

uint16_t compute_crc16(const uint8_t* data, size_t length) {
    uint16_t crc = 0x1337; // odrive docs
    for (size_t i = 0; i < length; ++i) {
        crc = (crc >> 8) ^ crc16_table[(crc ^ data[i]) & 0xFF];
    }
    return crc;
}

void send_odrive_command(HANDLE serial_handle, uint8_t axis, float torque) {
    uint8_t payload[5];
    payload[0] = axis;
    memcpy(&payload[1], &torque, sizeof(float)); // odrive docs

    uint8_t crc8 = compute_crc8(payload, sizeof(payload));

    uint8_t packet[1 + 1 + 1 + sizeof(payload) + 2]; // sync + length + crc8 + payload + crc16
    size_t offset = 0;
    packet[offset++] = 0xAA; // Sync byte
    packet[offset++] = static_cast<uint8_t>(sizeof(payload));
    packet[offset++] = crc8;
    memcpy(&packet[offset], payload, sizeof(payload));
    offset += sizeof(payload);
    uint16_t crc16 = compute_crc16(payload, sizeof(payload));
    memcpy(&packet[offset], &crc16, sizeof(crc16));
    offset += sizeof(crc16);

    // Asynchronous write
    OVERLAPPED ov = {0};
    ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    DWORD bytes_written;
    if (!WriteFile(serial_handle, packet, static_cast<DWORD>(offset), &bytes_written, &ov)) {
        if (GetLastError() == ERROR_IO_PENDING) {
            WaitForSingleObject(ov.hEvent, INFINITE);
            GetOverlappedResult(serial_handle, &ov, &bytes_written, FALSE);
        } else {
            std::cerr << "WriteFile failed with error: " << GetLastError() << std::endl;
        }
    }
    CloseHandle(ov.hEvent);
}

int main() {
    generate_crc8_table();
    generate_crc16_table();
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in udp_addr = {};
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(7777);
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    bind(udp_socket, (sockaddr*)&udp_addr, sizeof(udp_addr));

    // odrive serial port
    HANDLE serial_handle = CreateFileA("\\\\.\\COM5", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if (serial_handle == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open COM port" << std::endl;
        return 1;
    }

    DCB dcb = {};
    dcb.DCBlength = sizeof(dcb);
    GetCommState(serial_handle, &dcb);
    dcb.BaudRate = 115200;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;
    SetCommState(serial_handle, &dcb);

    // share between udp and control
    std::atomic<float> torque_l(0.0f), torque_r(0.0f);
    std::atomic<float> rumble1(0.0f), rumble2(0.0f);
    std::atomic<bool> running(true);

    std::thread udp_thread([&]() {
        uint8_t buffer[8];
        sockaddr_in sender_addr;
        int sender_len = sizeof(sender_addr);
        while (running) {
            int recv_len = recvfrom(udp_socket, (char*)buffer, sizeof(buffer), 0, (sockaddr*)&sender_addr, &sender_len);
            if (recv_len == sizeof(buffer)) {
                DEBUG_TIMESTAMP("UDP received");
                int16_t tl = buffer[0] | (buffer[1] << 8);
                int16_t tr = buffer[2] | (buffer[3] << 8);
                int16_t r1 = buffer[4] | (buffer[5] << 8);
                int16_t r2 = buffer[6] | (buffer[7] << 8);
                torque_l = static_cast<float>(tl) / 32768.0f;
                torque_r = static_cast<float>(tr) / 32768.0f;
                rumble1 = static_cast<float>(r1) / 32768.0f;
                rumble2 = static_cast<float>(r2) / 32768.0f;
            }
        }
    });

    std::thread control_thread([&]() {
        auto start_time = std::chrono::high_resolution_clock::now();
        while (running) {
            auto now = std::chrono::high_resolution_clock::now();
            float elapsed = std::chrono::duration<float>(now - start_time).count();

            // Generate sine wave from rumble inputs
            float freq1 = 50.0f + 50.0f * rumble1.load();
            float freq2 = 50.0f + 50.0f * rumble2.load();
            float amp1 = rumble1.load();
            float amp2 = rumble2.load();
            float sine_wave = amp1 * sinf(2.0f * 3.1415926f * freq1 * elapsed) +
                              amp2 * sinf(2.0f * 3.1415926f * freq2 * elapsed);

            // comglomerate !
            float final_torque_l = torque_l.load() + sine_wave;
            float final_torque_r = torque_r.load() + sine_wave;
            
            send_odrive_command(serial_handle, 0, final_torque_l);
            send_odrive_command(serial_handle, 1, final_torque_r);

            DEBUG_TIMESTAMP("Torque commands sent");

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    std::cout << "enter to terminate" << std::endl;
    std::cin.get();
    running = false;

    udp_thread.join();
    control_thread.join();

    CloseHandle(serial_handle);
    closesocket(udp_socket);
    WSACleanup();

    return 0;
}
