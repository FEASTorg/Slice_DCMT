// include/globals.h
#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <stdint.h>
#include <FastLED.h>
#include <crumbs.h>
#include "config_hardware.h"

// ---- Shared state structs ----
struct DCMT_SLICE
{
    int16_t motor1PWM = 0;
    int16_t motor2PWM = 0;
    bool motor1Brake = false;
    bool motor2Brake = false;
    bool eStop = false;
};

struct Timing
{
    long lastSerialPrint;
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
void reply_version(crumbs_context_t *ctx, crumbs_message_t *reply, void *user_data);
void reply_get_state(crumbs_context_t *ctx, crumbs_message_t *reply, void *user_data);

#endif // GLOBALS_H
