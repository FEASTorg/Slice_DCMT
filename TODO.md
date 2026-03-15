# TODO

## Closed-Loop Validation (DCMotorServo Backend)

Status: Runtime migration implemented; bench validation pending.

### Current Implementation

- Firmware now uses `DCMotorServo` + `DCMotorTacho` backend (archive-style control stack).
- Archive-proven CPR is applied (`MOTOR1_CPR` / `MOTOR2_CPR` = `798`).
- Speed-loop remains capability-gated by MCU profile (`DCMT_ENABLE_SPEED_LOOP` default logic).
- Wire contract remains unchanged (`bread-crumbs-contracts` DCMT ops + caps).

### Remaining Work Items

1. Build validation across environments:
   - `gen1_nano`
   - `gen2_nano`
   - `gen1_nanoevery`
   - `gen2_nanoevery`
2. Bench test and tune as needed for:
   - position control stability
   - speed control stability
   - brake/estop safety behavior during closed-loop operation
3. Controller protocol regression checks:
   - `version`
   - `caps`
   - `state`
   - `mode/setpoint/pid`
4. Confirm caps/behavior alignment:
   - Level 2: pos closed-loop available
   - Level 3: speed closed-loop available

### Acceptance Criteria

1. All four envs compile with no migration regressions.
2. Closed-loop command/telemetry behavior matches current `dcmt_ops.h` contract.
3. Bench validation passes for position, speed (where enabled), and safety paths.
