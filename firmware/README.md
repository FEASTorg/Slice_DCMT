# DCMT Firmware Profiles

This firmware separates three concerns:

1. Hardware generation (`DCMT_HW_GEN`)
- selects pin/electrical mapping only.

2. MCU performance profile
- derived from board target by default.
- classic Nano (AVR): speed loop disabled by default.
- Nano Every (megaAVR): speed loop enabled by default.

3. I2C address (`I2C_ADR`)
- independent from generation and board.
- override with `-DI2C_ADR=<addr>`.

## Environment Matrix

- `gen1_nano`
- `gen2_nano`
- `gen1_nanoevery`
- `gen2_nanoevery`

All environments support:

- open-loop
- closed-loop position

Speed closed-loop is controlled by `DCMT_ENABLE_SPEED_LOOP`.
Default behavior is board-dependent (see above), and can be overridden via build flag.

## Capability Mapping (`GET_CAPS`)

- Level 2: baseline + closed-position + PID tuning
- Level 3: Level 2 + closed-speed

Controllers should gate behavior by capability flags, not by generation labels.

## References

- Arduino Nano: https://store.arduino.cc/products/arduino-nano
- Arduino Nano Every: https://store.arduino.cc/products/nano-every
- Nano Every docs: https://docs.arduino.cc/hardware/nano-every
