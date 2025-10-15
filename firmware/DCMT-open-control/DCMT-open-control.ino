/**
 * @file DCMT.ino
 * @brief DCMT firmware for the BREAD system.
 * @author Cameron K. Brooks 2025
 *
 */

// DCMT specific libraries
#include <LMD18200.h> // LMD18200 motor driver library

// shared BREAD libs
#include <FastLED.h>

// BREAD specific libs
#include <CRUMBS.h>

// sketch specific libs
#include "config_hardware.h" // hardware configuration containing pin definitions
#include "config.h"          // configuration file for constants such as i2c address and timing

// CRUMBS start
CRUMBS crumbsSlice(false, I2C_ADR); // Peripheral mode

// flag for emergency stop
volatile bool estopTriggered = false;

// create object for the LED
CRGB led;

// Create first instance of the LMD18200 motor driver.
LMD18200 motor1Driver(MOTOR1_PWM_PIN, MOTOR1_DIR_PIN, MOTOR1_BRAKE_PIN, MOTOR1_CSENSE_PIN, MOTOR1_THERMAL_PIN);
// Create second instance of the LMD18200 motor driver.
LMD18200 motor2Driver(MOTOR2_PWM_PIN, MOTOR2_DIR_PIN, MOTOR2_BRAKE_PIN, MOTOR2_CSENSE_PIN, MOTOR2_THERMAL_PIN);

struct DCMT_SLICE
{
    ControlModes mode = OPEN_LOOP; // current mode of operation
    float motor1PWM = 0;           // motor 1 PWM value
    float motor2PWM = 0;           // motor 2 PWM value
    bool motor1Brake = false;      // motor 1 brake status
    bool motor2Brake = false;      // motor 2 brake status
    bool eStop = false;            // emergency stop flag
} slice;

struct Timing
{
    long lastSerialPrint;
} timing = {0};

void setup()
{
    // the general slice setup
    setupSlice();

    // the DCMT specific setup
    setupDCMT();

    // small delay before entering loop
    delay(1000);
}

void setupSlice()
{
    // initialize serial communication
    Serial.begin(115200);

    // Initialize CRUMBS communication
    crumbsSlice.begin();

    crumbsSlice.onRequest(handleRequest); // Register callback for data requests
    crumbsSlice.onReceive(handleMessage); // Register callback for received messages

    // initialize the LED
    FastLED.addLeds<NEOPIXEL, LED_PIN>(&led, 1);
    FastLED.setBrightness(50); // Set brightness to 50%
    led = CRGB::Blue;          // Set LED to blue
    FastLED.show();

    // initialize the estop
    pinMode(ESTOP, INPUT);
    attachInterrupt(digitalPinToInterrupt(ESTOP), estopISR, CHANGE);

    SLICE_DEBUG_PRINTLN(F("DCMT SLICE INITIALIZED"));
    SLICE_DEBUG_PRINTLN(F("VERSION: " VERSION));
}

void setupDCMT()
{
    // initialize the motor drivers
    motor1Driver.begin();
    motor2Driver.begin();
    motor1Driver.write(0);
    motor2Driver.write(0);
}

#define SLICE_PRINT_SERIAL 1 // set to 1 to enable serial printing for debugging

void loop()
{
    // poll the emergency stop
    pollEStop();

    // update the motor control logic
    motorControlLogic();

    // serial commands input
    serialCommands();

    // serial print output
    printSerialOutput();
}

void pollEStop()
{
    if (estopTriggered)
    {
        // Update LED to indicate emergency stop
        led = CRGB::Red;
        FastLED.show();
        processEStop();
        estopTriggered = false; // Reset flag after handling
    }
    else
    {
        // Update LED to indicate normal operation
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
    // Read the current state of the estop pin if needed for debouncing/validation
    if (digitalRead(ESTOP) == HIGH)
    {
        // Disable the system if the estop is pressed
        led = CRGB::Red;
        FastLED.show();

        motor1Driver.brake(); // Brake motor 1
        motor2Driver.brake(); // Brake motor 2

        // Set the emergency stop flag in the system
        slice.eStop = true;
        SLICE_DEBUG_PRINTLN("ESTOP PRESSED!");
    }
    else
    {
        // Re-enable the system if the estop is released
        led = CRGB::Green;
        FastLED.show();
        slice.eStop = false;
        SLICE_DEBUG_PRINTLN("ESTOP RELEASED!");
    }
}

void motorControlLogic()
{
    if (slice.motor1Brake || slice.motor2Brake) // Process brakes
    {
        // Handle motor 1 brake
        if (slice.motor1Brake)
        {
            slice.motor1PWM = 0;
            motor1Driver.write(0);
            motor1Driver.brake();
        }
        // Handle motor 2 brake
        if (slice.motor2Brake)
        {
            slice.motor2PWM = 0;
            motor2Driver.write(0);
            motor2Driver.brake();
        }
    }
    else if (!slice.eStop && slice.mode == OPEN_LOOP) // Process open loop
    {
        // Update the PWM values for the motors
        motor1Driver.write(slice.motor1PWM);
        motor2Driver.write(slice.motor2PWM);
    }
    else // unknown mode
    {
        // soft stop by setting brakes
        slice.motor1Brake = true;
        slice.motor2Brake = true;
        SLICE_DEBUG_PRINTLN(F("ERROR INVALID OPERATION MODE, ENTERED ERROR OR UNKNOWN STATE!"));
    }
}
