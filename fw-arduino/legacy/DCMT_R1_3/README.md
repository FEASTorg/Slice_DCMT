# RLHT R1.3 Firmware Docs

Primary change from R1.2 --> R1.3 is the addition of closed loop control using [DCMotorServo](https://github.com/CameronBrooks11/DCMotorServo)

## Serial Command Usage

Send commands via serial using the following formats:

- **Mode Switch (`M` Command):**  
  Format: `M,<mode>`  
  Example: `M,0` (switches to CONTROL mode), `M,1` (switches to WRITE mode)

- **Motor Update (`W` Command):**  
  Format: `W,<motorSelect>,<speedSelect>`  
  Example: `W,1,100` (sets motor 1 speed to 100%), `W,2,-50` (sets motor 2 speed to 50% in reverse)
