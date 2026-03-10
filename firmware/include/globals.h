// include/globals.h
#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <stdint.h>
#include <FastLED.h>
#include <crumbs.h>
#include "config_hardware.h"

enum ControlModes : uint8_t
{
    OPEN_LOOP = 0,
    CLOSED_LOOP_POSITION = 1,
    CLOSED_LOOP_SPEED = 2
};

struct PIDTunings
{
    float kp = 1.0f;
    float ki = 0.0f;
    float kd = 0.0f;
};

struct PIDState
{
    float integral = 0.0f;
    float previousError = 0.0f;
};

// ---- Shared state structs ----
struct DCMT_SLICE
{
    ControlModes mode = OPEN_LOOP;
    int16_t motor1PWM = 0;
    int16_t motor2PWM = 0;
    bool motor1Brake = false;
    bool motor2Brake = false;
    bool eStop = false;

#if DCMT_FEATURE_CLOSED_LOOP
    int16_t motor1PositionSetpoint = 0;
    int16_t motor2PositionSetpoint = 0;
    int16_t motor1Position = 0;
    int16_t motor2Position = 0;
    int16_t motor1SpeedSetpoint = 0;
    int16_t motor2SpeedSetpoint = 0;
    int16_t motor1Speed = 0;
    int16_t motor2Speed = 0;

    PIDTunings posPid1 = {2.0f, 0.0f, 0.05f};
    PIDTunings posPid2 = {2.0f, 0.0f, 0.05f};
    PIDTunings speedPid1 = {0.6f, 0.02f, 0.0f};
    PIDTunings speedPid2 = {0.6f, 0.02f, 0.0f};
    PIDState posState1 = {};
    PIDState posState2 = {};
    PIDState speedState1 = {};
    PIDState speedState2 = {};
#endif
};

struct Timing
{
    long lastSerialPrint;
    long lastControlUpdate;
#if DCMT_FEATURE_CLOSED_LOOP
    long lastSpeedSample;
#endif
};

// ---- Extern globals (defined in main.cpp) ----
extern volatile bool estopTriggered;
extern CRGB led;
extern DCMT_SLICE slice;
extern Timing timing;

// ---- Functions implemented across translation units ----
void setupSlice();
void setupDCMT();
void pollEStop();
void estopISR();
void processEStop();
void motorControlLogic();
void printSerialOutput();
void serialCommands();
void handler_set_open_loop(crumbs_context_t *ctx, uint8_t opcode, const uint8_t *data, uint8_t data_len, void *user_data);
void handler_set_brake(crumbs_context_t *ctx, uint8_t opcode, const uint8_t *data, uint8_t data_len, void *user_data);
void handler_set_mode(crumbs_context_t *ctx, uint8_t opcode, const uint8_t *data, uint8_t data_len, void *user_data);
void handler_set_setpoint(crumbs_context_t *ctx, uint8_t opcode, const uint8_t *data, uint8_t data_len, void *user_data);
void handler_set_pid(crumbs_context_t *ctx, uint8_t opcode, const uint8_t *data, uint8_t data_len, void *user_data);
void reply_version(crumbs_context_t *ctx, crumbs_message_t *reply, void *user_data);
void reply_get_state(crumbs_context_t *ctx, crumbs_message_t *reply, void *user_data);

#endif // GLOBALS_H
