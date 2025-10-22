# RLHT R1.2 Firmware Docs

Primary change from R1.1 --> R1.2 is the usage of the LMD lib.

## Serial Command Usage

Send commands via serial using the following formats:

- **Mode Switch (`M` Command):**  
  Format: `M,<mode>`  
  Example: `M,0` (switches to CONTROL mode), `M,1` (switches to WRITE mode)

- **Motor Update (`W` Command):**  
  Format: `W,<motorSelect>,<speedSelect>`  
  Example: `W,1,100` (sets motor 1 speed to 100%), `W,2,-50` (sets motor 2 speed to 50% in reverse)
