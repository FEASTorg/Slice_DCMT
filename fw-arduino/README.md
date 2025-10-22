# Firmware

This directory contains the firmware for the DC Motorized Translation (DCMT) stage. There are three versions of the firmware available:

1. **DCMT-old**: This is the original firmware version used in earlier iterations of the DCMT stage. It provides basic functionality for controlling the motorized stage.

2. **DCMT-open-control**: This version of the firmware allows for open-loop control of the DCMT stage. It provides improved performance and additional features compared to the old version.

3. **DCMT-closed-control**: This is the most advanced version of the firmware, offering closed-loop control for enhanced precision and reliability. It includes features such as position feedback and error correction.

## Development

Always ensure you update the version number in `config.h` when making changes to the firmware and update it's changelog. This helps in tracking different versions and ensuring compatibility with other components of the system.
