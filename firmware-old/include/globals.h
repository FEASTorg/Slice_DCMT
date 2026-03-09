// include/globals.h
#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <FastLED.h>
#include <CRUMBS.h>
#include "config_hardware.h"

// ---- Shared state structs ----
struct DCMT_SLICE
{
    ControlModes mode = OPEN_LOOP;
    float motor1PWM = 0;
    float motor2PWM = 0;
    bool motor1Brake = false;
    bool motor2Brake = false;
    bool eStop = false;
};

struct Timing
{
    long lastSerialPrint;
};

// ---- Extern globals (defined in main.cpp) ----
extern CRUMBS crumbsSlice;
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
void handleMessage(CRUMBSMessage &message);
void handleRequest();
void printSerialOutput();
void serialCommands();

#endif // GLOBALS_H
