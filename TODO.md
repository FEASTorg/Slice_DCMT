# TODO

## Deferred: Replace Temporary Closed-Loop Runtime with DCMotorServo Stack

Status: Deferred for later task.

### Why

- Current firmware uses an in-firmware PID implementation (`Encoder` + custom control loop).
- Speed-loop availability is currently gated by MCU performance profile (default enabled on Nano Every, disabled on classic Nano).
- Original closed-loop behavior used `DCMotorServo` + `DCMotorTacho`.
- We want eventual parity with the legacy closed-loop architecture and tuning model.

### Future Work Items

1. Add `DCMotorServo` and `DCMotorTacho` dependencies in `firmware/platformio.ini`.
2. Replace custom closed-loop runtime in `firmware/src/main.cpp` with library-backed control objects.
3. Map CRUMBS closed-loop commands to library APIs:
   - mode switch
   - setpoint updates
   - PID tuning updates
4. Validate telemetry reply format in `firmware/src/dcmt_handlers.cpp` against expected controller contract.
5. Re-run bench tests for:
   - position control stability
   - speed control stability
   - brake/estop safety behavior during closed-loop operation

### Acceptance Criteria

1. `gen1_nano`, `gen2_nano`, `gen1_nanoevery`, and `gen2_nanoevery` compile and run using `DCMotorServo`/`DCMotorTacho` (no custom PID loop active).
2. Closed-loop command/telemetry behavior matches intended Gen2 contract.
3. Bench validation passes for position, speed, and safety paths.
