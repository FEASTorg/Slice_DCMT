# RLHT R1.1 Firmware Docs

Notes on the firmware for dc motor controller slice.

## Serial Command Usage

Send commands via serial using the following formats:

- **Mode Switch (`M` Command):**  
  Format: `M,<mode>`  
  Example: `M,0` (switches to CONTROL mode), `M,1` (switches to WRITE mode)

- **Motor Update (`W` Command):**  
  Format: `W,<motorSelect>,<speedSelect>`  
  Example: `W,1,100` (sets motor 1 speed to 100%), `W,2,-50` (sets motor 2 speed to 50% in reverse)
