/**
 * @file RLHT.ino
 * @brief RLHT firmware for the BREAD system.
 * @author Cameron K. Brooks 2025
 *
 */

// DCMT specific libraries
// #include <DCMotorServo.h>  // library using PID and encoder feedback for DC motor control

// shared BREAD libs
#include <FastLED.h>

// BREAD specific libs
#include <CRUMBS.h>

// sketch specific libs
#include "config_hardware.h" // hardware configuration containing pin definitions
#include "config.h"          // configuration file for constants such as i2c address and timing

// CRUMBS start
CRUMBS crumbsSlice(false, I2C_ADR); // Peripheral mode, I2C address 0x08

// flag for emergency stop
volatile bool estopTriggered = false;

// create object for the LED
CRGB led;

struct RelayHeater
{
    double setpointTemperature = 0; // set desired temperature for heater 1 (if using relay as heater actuator)
    int thermocoupleSelect = 0;     // select which thermocouple to use for the relay
    double inputTemperature = 0;    // input to relay 1 taken from selected thermocouple
    double relayOnTime = 0;         // time relay is on in ms (0-period)
    int relayPeriod = 1000;         // duration of relay 1 cycle in ms (1000ms = 1s)
    double Kp = 1;                  // PID proportional gain for heater 1
    double Ki = 0;                  // PID integral gain for heater 1
    double Kd = 0;                  // PID derivative gain for heater 1
};

struct RLHT_SLICE
{
    double temperature1;      // thermocouple 1 measurement
    double temperature2;      // thermocouple 2 measurement
    RelayHeater relayHeater1; // relay heater 1
    RelayHeater relayHeater2; // relay heater 2
    bool eStop = false;       // emergency stop flag
} slice;

struct Timing
{
    long lastThermoRead;
    long lastSerialPrint;
    unsigned long relay1Start;
    unsigned long relay2Start;
} timing = {0};

enum ControlMode
{
    CLOSED_LOOP,
    OPEN_LOOP
} currentMode = CLOSED_LOOP;

// initialize the Thermocouples
MAX6675 thermocouple1(TC_CLK, TC_CS1, TC_DATA);
MAX6675 thermocouple2(TC_CLK, TC_CS2, TC_DATA);

// Specify the links and initial tuning parameters
// PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
PID relay1PID(&(slice.relayHeater1.inputTemperature), &(slice.relayHeater1.relayOnTime), &(slice.relayHeater1.setpointTemperature), slice.relayHeater1.Kp, slice.relayHeater1.Ki, slice.relayHeater1.Kd, DIRECT);
PID relay2PID(&(slice.relayHeater2.inputTemperature), &(slice.relayHeater2.relayOnTime), &(slice.relayHeater2.setpointTemperature), slice.relayHeater2.Kp, slice.relayHeater2.Ki, slice.relayHeater2.Kd, DIRECT);

void setup()
{
    // the general slice setup
    setupSlice();

    // the RLHT specific setup
    setupRLHT();

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

    // initialize the estop
    pinMode(ESTOP, INPUT);
    attachInterrupt(digitalPinToInterrupt(ESTOP), estopISR, CHANGE);
}

void setupRLHT()
{
    // initialize the relay pins
    pinMode(RELAY1, OUTPUT);
    pinMode(RELAY2, OUTPUT);

    // tell the PID to range between 0 and the full window size
    relay1PID.SetOutputLimits(0, slice.relayHeater1.relayPeriod);
    relay2PID.SetOutputLimits(0, slice.relayHeater2.relayPeriod);

    // turn the PID on
    relay1PID.SetMode(AUTOMATIC);
    relay2PID.SetMode(AUTOMATIC);

    // initialize the relay start times
    timing.relay1Start = millis();
    timing.relay2Start = millis();
}

void loop()
{

    // poll the emergency stop
    pollEStop();

    // read the thermocouples
    measureThermocouples();

    // main control logic
    relayControlLogic();

    // print output to serial
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
        // Disable PID and turn off relays safely
        slice.relayHeater1.setpointTemperature = 0;
        slice.relayHeater2.setpointTemperature = 0;
        slice.relayHeater1.relayOnTime = 0;
        slice.relayHeater2.relayOnTime = 0;
        digitalWrite(RELAY1, LOW);
        digitalWrite(RELAY2, LOW);

        // Set the emergency stop flag in the system
        slice.eStop = true;
        SLICE_DEBUG_PRINTLN("ESTOP PRESSED!");
    }
    else
    {
        // If the button/switch is released, restore or handle recovery
        led = CRGB::Green;
        FastLED.show();

        // Optionally restore previous settings or require a manual reset
        slice.eStop = false;
        SLICE_DEBUG_PRINTLN("ESTOP RELEASED!");
    }
}

void measureThermocouples()
{
    if (millis() - timing.lastThermoRead >= THERMO_UPDATE_TIME_MS)
    {
        slice.temperature1 = thermocouple1.readCelsius();
        slice.temperature2 = thermocouple2.readCelsius();
        timing.lastThermoRead = millis();
    }
}

void relayControlLogic()
{

    if (!slice.eStop && currentMode == CLOSED_LOOP)
    {
        // ensure the PID is on
        relay1PID.SetMode(AUTOMATIC);
        relay2PID.SetMode(AUTOMATIC);

        // set the PID tunings
        relay1PID.SetTunings(slice.relayHeater1.Kp, slice.relayHeater1.Ki, slice.relayHeater1.Kd);
        relay2PID.SetTunings(slice.relayHeater2.Kp, slice.relayHeater2.Ki, slice.relayHeater2.Kd);

        // assign thermocouple readings to relay inputs
        switch (slice.relayHeater1.thermocoupleSelect)
        {
        case 1: // thermocouple 1 to relay 1
            slice.relayHeater1.inputTemperature = slice.temperature1;
            break;
        case 2: // thermocouple 2 to relay 1
            slice.relayHeater1.inputTemperature = slice.temperature2;
            break;
        }
        if (isnan(slice.relayHeater1.inputTemperature))
            slice.relayHeater1.relayOnTime = 0;
        else
            relay1PID.Compute();

        switch (slice.relayHeater2.thermocoupleSelect)
        {
        case 1: // thermocouple 1 to relay 2
            slice.relayHeater2.inputTemperature = slice.temperature1;
            break;
        case 2: // thermocouple 2 to relay 2
            slice.relayHeater2.inputTemperature = slice.temperature2;
            break;
        }
        if (isnan(slice.relayHeater2.inputTemperature))
            slice.relayHeater2.relayOnTime = 0;
        else
            relay2PID.Compute();

        // actuate the relays
        actuateRelay(RELAY1, timing.relay1Start, slice.relayHeater1.relayPeriod, slice.relayHeater1.relayOnTime);
        actuateRelay(RELAY2, timing.relay2Start, slice.relayHeater2.relayPeriod, slice.relayHeater2.relayOnTime);
    }
    else if (!slice.eStop && currentMode == OPEN_LOOP)
    {
        // ensure the PID controller is turned off
        relay1PID.SetMode(MANUAL);
        relay2PID.SetMode(MANUAL);

        // actuate the relays
        actuateRelay(RELAY1, timing.relay1Start, slice.relayHeater1.relayPeriod, slice.relayHeater1.relayOnTime);
        actuateRelay(RELAY2, timing.relay2Start, slice.relayHeater2.relayPeriod, slice.relayHeater2.relayOnTime);
    }
    else
    {
        // turn off the relays
        digitalWrite(RELAY1, LOW);
        digitalWrite(RELAY2, LOW);
        SLICE_DEBUG_PRINTLN(F("ERROR INVALID MODE, ENTERED UNKNOWN STATE!"));
    }
}

void actuateRelay(uint8_t relayPin, unsigned long &relayStart, unsigned long relayPeriod, unsigned long relayOnTime)
{
    unsigned long currentTime = millis();

    // Shift relay window if period has elapsed
    if (currentTime - relayStart > relayPeriod)
    {
        relayStart += relayPeriod;
    }

    // Set relay state based on ON duration
    digitalWrite(relayPin, (currentTime - relayStart) < relayOnTime ? HIGH : LOW);
}