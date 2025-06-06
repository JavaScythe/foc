import odrive
from odrive.enums import *
import time

print("Finding ODrive...")
odrv0 = odrive.find_any()
print("Connected.")

omotor = odrv0.axis0

print("Configuring motor...")
omotor.motor.config.pole_pairs = 4
omotor.motor.config.current_lim = 58
omotor.motor.config.calibration_current = 15
omotor.motor.config.torque_constant = 0.12 
omotor.motor.config.pre_calibrated = False

print("Configuring encoder...")
omotor.encoder.config.mode = ENCODER_MODE_INCREMENTAL
omotor.encoder.config.cpr = 8192 # Change
omotor.encoder.config.use_index = True
omotor.encoder.config.pre_calibrated = False

omotor.config.startup_motor_calibration = False

omotor.config.startup_encoder_index_search = True
omotor.encoder.config.index_search_unidirectional = True
omotor.encoder.config.calib_scan_distance = 8192 # Change to 2-3x CPR
print("Configuring autoindex for SPECIFIC DIRECTION, double check");
omotor.config.calibration_lockin.vel = +1

omotor.config.startup_encoder_offset_calibration = False
omotor.config.startup_closed_loop_control = True

print("Running full calibration sequence...")
omotor.requested_state = AXIS_STATE_FULL_CALIBRATION_SEQUENCE
while omotor.current_state != AXIS_STATE_IDLE:
    time.sleep(0.1)
print("Calibration complete.")

omotor.motor.config.pre_calibrated = True
omotor.encoder.config.pre_calibrated = True

print("Setting torque control mode...")
omotor.controller.config.control_mode = CONTROL_MODE_TORQUE_CONTROL
omotor.controller.config.input_mode = INPUT_MODE_PASSTHROUGH

# safety
omotor.controller.enable_torque_mode_vel_limit = False
omotor.controller.config.vel_limit = 10000
omotor.trap_traj.config.accel_limit = 10000
omotor.trap_traj.config.decel_limit = 10000

print("Running index search...")
omotor.requested_state = AXIS_STATE_ENCODER_INDEX_SEARCH
while omotor.current_state != AXIS_STATE_IDLE:
    time.sleep(0.1)

print("Running encoder offset calibration...")
omotor.requested_state = AXIS_STATE_ENCODER_OFFSET_CALIBRATION
while omotor.current_state != AXIS_STATE_IDLE:
    time.sleep(0.1)

print("Entering closed-loop control...")
omotor.requested_state = AXIS_STATE_CLOSED_LOOP_CONTROL

print("Applying test torque...")
for i in range(3):
    omotor.controller.input_torque = 0.05  # Nm
    time.sleep(1)
    omotor.controller.input_torque = -0.05
    time.sleep(1)

omotor.controller.input_torque = 0
print("Torque test complete.")
print("Saving configuration...")
odrv0.config.usb_cdc_protocol = 0x0;
odrv0.save_configuration();
print("Done.")
