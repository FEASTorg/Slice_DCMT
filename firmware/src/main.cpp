// src/main.cpp
/**
 * @file main.cpp
 * @brief DCMT firmware (PlatformIO) for the BREAD system.
 */

#include <Arduino.h>
#include <FastLED.h>
#include <crumbs.h>
#include <crumbs_arduino.h>
#include <LMD18200.h>
#include <dcmt_ops.h>

#include "config.h"
#include "config_hardware.h"
#include "globals.h"

// ---- CRUMBS context ----
static crumbs_context_t ctx;

// ---- Global flags/objects ----
volatile bool estopTriggered = false;
CRGB led;

// Motor drivers
LMD18200 motor1Driver(MOTOR1_PWM_PIN, MOTOR1_DIR_PIN, MOTOR1_BRAKE_PIN, MOTOR1_CSENSE_PIN);
LMD18200 motor2Driver(MOTOR2_PWM_PIN, MOTOR2_DIR_PIN, MOTOR2_BRAKE_PIN, MOTOR2_CSENSE_PIN);

// Shared state
DCMT_SLICE slice;
Timing timing = {0};

void setup()
{
    setupSlice();
    setupDCMT();
    delay(1000);
}

void loop()
{
    pollEStop();
    motorControlLogic();
    serialCommands();
    printSerialOutput();
}

// ---- Implementation (previously in .ino files) ----

void setupSlice()
{
    int rc;
    Serial.begin(115200);

    // CRUMBS
    crumbs_arduino_init_peripheral(&ctx, I2C_ADR);

    rc = crumbs_register_handler(&ctx, DCMT_OP_SET_OPEN_LOOP, handler_set_open_loop, nullptr);
    if (rc != 0)
        SLICE_DEBUG_PRINTLN(F("CRUMBS: Failed to register DCMT_OP_SET_OPEN_LOOP"));
    rc = crumbs_register_handler(&ctx, DCMT_OP_SET_BRAKE, handler_set_brake, nullptr);
    if (rc != 0)
        SLICE_DEBUG_PRINTLN(F("CRUMBS: Failed to register DCMT_OP_SET_BRAKE"));
    rc = crumbs_register_reply_handler(&ctx, 0x00, reply_version, nullptr);
    if (rc != 0)
        SLICE_DEBUG_PRINTLN(F("CRUMBS: Failed to register version reply handler"));
    rc = crumbs_register_reply_handler(&ctx, DCMT_OP_GET_STATE, reply_get_state, nullptr);
    if (rc != 0)
        SLICE_DEBUG_PRINTLN(F("CRUMBS: Failed to register DCMT_OP_GET_STATE reply handler"));

    // LED
    FastLED.addLeds<NEOPIXEL, LED_PIN>(&led, 1);
    FastLED.setBrightness(50);
    led = CRGB::Blue;
    FastLED.show();

    // e-stop
    pinMode(ESTOP, INPUT);
    attachInterrupt(digitalPinToInterrupt(ESTOP), estopISR, CHANGE);

    SLICE_DEBUG_PRINTLN(F("DCMT SLICE INITIALIZED"));
    SLICE_DEBUG_PRINTLN(F("VERSION: " VERSION));
}

void setupDCMT()
{
    motor1Driver.begin();
    motor2Driver.begin();
    motor1Driver.write(0);
    motor2Driver.write(0);
}

// Poll/ISR/processing
void pollEStop()
{
    if (estopTriggered)
    {
        led = CRGB::Red;
        FastLED.show();
        processEStop();
        estopTriggered = false;
    }
    else
    {
        led = CRGB::Green;
        FastLED.show();
    }
}

void estopISR()
{
    estopTriggered = true;
}

void processEStop()
{
    if (digitalRead(ESTOP) == HIGH)
    {
        led = CRGB::Red;
        FastLED.show();
        motor1Driver.brake();
        motor2Driver.brake();
        slice.eStop = true;
        SLICE_DEBUG_PRINTLN(F("ESTOP PRESSED!"));
    }
    else
    {
        led = CRGB::Green;
        FastLED.show();
        slice.eStop = false;
        SLICE_DEBUG_PRINTLN(F("ESTOP RELEASED!"));
    }
}

void motorControlLogic()
{
    if (slice.motor1Brake || slice.motor2Brake)
    {
        if (slice.motor1Brake)
        {
            slice.motor1PWM = 0;
            motor1Driver.write(0);
            motor1Driver.brake();
        }
        if (slice.motor2Brake)
        {
            slice.motor2PWM = 0;
            motor2Driver.write(0);
            motor2Driver.brake();
        }
    }
    else if (!slice.eStop)
    {
        motor1Driver.write(slice.motor1PWM);
        motor2Driver.write(slice.motor2PWM);
    }
    else
    {
        // unknown/invalid -> soft stop
        slice.motor1Brake = true;
        slice.motor2Brake = true;
        SLICE_DEBUG_PRINTLN(F("ERROR INVALID OPERATION MODE, ENTERED ERROR OR UNKNOWN STATE!"));
    }
}
