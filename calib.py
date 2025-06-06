import odrive
from odrive.enums import *
import time

print("Finding ODrive...")
odrv0 = odrive.find_any()
print("Connected.")

print("Configuring motor...")
odrv0.axis0.motor.config.pole_pairs = 4
odrv0.axis0.motor.config.current_lim = 58
odrv0.axis0.motor.config.calibration_current = 15
odrv0.axis0.motor.config.torque_constant = 0.12 
odrv0.axis0.motor.config.pre_calibrated = False

print("Configuring encoder...")
odrv0.axis0.encoder.config.mode = ENCODER_MODE_INCREMENTAL
odrv0.axis0.encoder.config.cpr = 8192 # Change
odrv0.axis0.encoder.config.use_index = True
odrv0.axis0.encoder.config.pre_calibrated = False

odrv0.axis0.config.startup_motor_calibration = False

odrv0.axis0.config.startup_encoder_index_search = True
odrv0.axis0.encoder.config.index_search_unidirectional = True
odrv0.axis0.encoder.config.calib_scan_distance = 8192 # Change to 2-3x CPR
print("Configuring autoindex for SPECIFIC DIRECTION, double check");
odrv0.axis0.config.calibration_lockin.vel = +1

odrv0.axis0.config.startup_encoder_offset_calibration = False
odrv0.axis0.config.startup_closed_loop_control = True

print("Running full calibration sequence...")
odrv0.axis0.requested_state = AXIS_STATE_FULL_CALIBRATION_SEQUENCE
while odrv0.axis0.current_state != AXIS_STATE_IDLE:
    time.sleep(0.1)
print("Calibration complete.")

odrv0.axis0.motor.config.pre_calibrated = True
odrv0.axis0.encoder.config.pre_calibrated = True

print("Setting torque control mode...")
odrv0.axis0.controller.config.control_mode = CONTROL_MODE_TORQUE_CONTROL
odrv0.axis0.controller.config.input_mode = INPUT_MODE_PASSTHROUGH

# safety
odrv0.axis0.controller.enable_torque_mode_vel_limit = False
odrv0.axis0.controller.config.vel_limit = 10000
odrv0.axis0.trap_traj.config.accel_limit = 10000
odrv0.axis0.trap_traj.config.decel_limit = 10000

print("Running index search...")
odrv0.axis0.requested_state = AXIS_STATE_ENCODER_INDEX_SEARCH
while odrv0.axis0.current_state != AXIS_STATE_IDLE:
    time.sleep(0.1)

print("Running encoder offset calibration...")
odrv0.axis0.requested_state = AXIS_STATE_ENCODER_OFFSET_CALIBRATION
while odrv0.axis0.current_state != AXIS_STATE_IDLE:
    time.sleep(0.1)

print("Entering closed-loop control...")
odrv0.axis0.requested_state = AXIS_STATE_CLOSED_LOOP_CONTROL

print("Applying test torque...")
for i in range(3):
    odrv0.axis0.controller.input_torque = 0.05  # Nm
    time.sleep(1)
    odrv0.axis0.controller.input_torque = -0.05
    time.sleep(1)

odrv0.axis0.controller.input_torque = 0
print("Torque test complete.")
print("Saving configuration...")
odrv0.config.usb_cdc_protocol = 0x0;
odrv0.save_configuration()
print("Done.")
