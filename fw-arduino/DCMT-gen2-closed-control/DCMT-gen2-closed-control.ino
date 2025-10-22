/**
 * @file DCMT.ino
 * @brief DCMT firmware for the BREAD system.
 * @author Cameron K. Brooks 2025
 *
 */

// DCMT specific libraries
#include <LMD18200.h>     // LMD18200 motor driver library
#include <Encoder.h>      // Encoder library for reading motor position
#include <DCMotorServo.h> // library using PID and encoder feedback for DC motor control
#include <DCMotorTacho.h> // library using PID and tachometer feedback for DC motor control

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

// Create first instance of the LMD18200 motor driver.
LMD18200 motor1Driver(MOTOR1_PWM_PIN, MOTOR1_DIR_PIN, MOTOR1_BRAKE_PIN, MOTOR1_CSENSE_PIN, MOTOR1_THERMAL_PIN);
// Create second instance of the LMD18200 motor driver.
LMD18200 motor2Driver(MOTOR2_PWM_PIN, MOTOR2_DIR_PIN, MOTOR2_BRAKE_PIN, MOTOR2_CSENSE_PIN, MOTOR2_THERMAL_PIN);

// Create first instance of the encoder.
Encoder motor1Encoder(MOTOR1_ENCODER_PIN1, MOTOR1_ENCODER_PIN2);
// Create second instance of the encoder.
Encoder motor2Encoder(MOTOR2_ENCODER_PIN1, MOTOR2_ENCODER_PIN2);

// Wrapper functions for the first motor driver and encoder.
void lmdMotor1Write(int16_t speed)
{
    motor1Driver.write(speed);
}
void lmdMotor1Brake()
{
    motor1Driver.brake();
}
long encoder1ReadFunc()
{
    return motor1Encoder.read();
}
void encoder1WriteFunc(long newPosition)
{
    motor1Encoder.write(newPosition);
}

// Wrapper functions for the second motor driver and encoder.
void lmdMotor2Write(int16_t speed)
{
    motor2Driver.write(speed);
}
void lmdMotor2Brake()
{
    motor2Driver.brake();
}
long encoder2ReadFunc()
{
    return motor2Encoder.read();
}
void encoder2WriteFunc(long newPosition)
{
    motor2Encoder.write(newPosition);
}

#define MOTOR1_PWM_SKIP 15 // PWM skip for motor 1
#define MOTOR1_PWM_MAX 150 // Max PWM for motor 1
#define MOTOR1_ACCURACY 10 // Accuracy for motor 1
DCMotorServo servo1(lmdMotor1Write, lmdMotor1Brake, encoder1ReadFunc, encoder1WriteFunc);

#define MOTOR2_PWM_SKIP 15 // PWM skip for motor 2
#define MOTOR2_PWM_MAX 150 // Max PWM for motor 2
#define MOTOR2_ACCURACY 10 // Accuracy for motor 2
DCMotorServo servo2(lmdMotor2Write, lmdMotor2Brake, encoder2ReadFunc, encoder2WriteFunc);

// Physical parameters for motor1 rotational and speed control.
#define MOTOR1_PPR 12
#define MOTOR1_GEAR_RATIO 19
#define MOTOR1_EMPIRICAL_FUDGE_FACTOR 0.875
#define MOTOR1_ENCODER_RESOLUTION (MOTOR1_PPR * MOTOR1_GEAR_RATIO)
#define MOTOR1_CPR (MOTOR1_ENCODER_RESOLUTION * 4 * MOTOR1_EMPIRICAL_FUDGE_FACTOR)

#define MOTOR1_SPEED_INTERVAL 5 // ms

// Create first instance of the DCMotorTacho class.
DCMotorTacho tacho1(&servo1, MOTOR1_CPR, MOTOR1_SPEED_INTERVAL);

// Physical parameters for motor2 rotational and speed control.
#define MOTOR2_PPR 12
#define MOTOR2_GEAR_RATIO 19
#define MOTOR2_EMPIRICAL_FUDGE_FACTOR 0.875
#define MOTOR2_ENCODER_RESOLUTION (MOTOR2_PPR * MOTOR2_GEAR_RATIO)
#define MOTOR2_CPR (MOTOR2_ENCODER_RESOLUTION * 4 * MOTOR2_EMPIRICAL_FUDGE_FACTOR)

#define MOTOR2_SPEED_INTERVAL 5 // ms

// Create first instance of the DCMotorTacho class.
DCMotorTacho tacho2(&servo2, MOTOR2_CPR, MOTOR2_SPEED_INTERVAL);

float motor1OuterKp = 0.15, motor1OuterKi = 0.01, motor1OuterKd = 0.01;
float motor1InnerKp = 0.15, motor1InnerKi = 0.01, motor1InnerKd = 0.01;
float motor2OuterKp = 0.15, motor2OuterKi = 0.01, motor2OuterKd = 0.01;
float motor2InnerKp = 0.15, motor2InnerKi = 0.01, motor2InnerKd = 0.01;

struct DCMT_SLICE
{
    ControlModes mode = OPEN_LOOP;    // current mode of operation
    float motor1PWM = 0;              // motor 1 PWM value
    float motor2PWM = 0;              // motor 2 PWM value
    float motor1PositionSetpoint = 0; // motor 1 position setpoint
    float motor2PositionSetpoint = 0; // motor 2 position setpoint
    float motor1Position = 0;         // motor 1 position
    float motor2Position = 0;         // motor 2 position
    float motor1SpeedSetpoint = 0;    // motor 1 speed setpoint
    float motor2SpeedSetpoint = 0;    // motor 2 speed setpoint
    float motor1Speed = 0;            // motor 1 speed
    float motor2Speed = 0;            // motor 2 speed
    bool motor1Brake = false;         // motor 1 brake status
    bool motor2Brake = false;         // motor 2 brake status
    bool eStop = false;               // emergency stop flag
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

    // set the outer (position) loop PID controllers
    servo1.setPIDTunings(motor1OuterKp, motor1OuterKi, motor1OuterKd);
    servo2.setPIDTunings(motor2OuterKp, motor2OuterKi, motor2OuterKd);

    // set pwm skip
    servo1.setPWMSkip(MOTOR1_PWM_SKIP);
    servo2.setPWMSkip(MOTOR2_PWM_SKIP);

    // set max pwm
    servo1.setMaxPWM(MOTOR1_PWM_MAX);
    servo2.setMaxPWM(MOTOR2_PWM_MAX);

    // set the accuracy
    servo1.setAccuracy(MOTOR1_ACCURACY);
    servo2.setAccuracy(MOTOR2_ACCURACY);

    // set tacho speed to zero
    tacho1.setSpeedRPM(0);
    tacho2.setSpeedRPM(0);

    // set the inner (speed) loop PID controllers
    tacho1.setPIDTunings(motor1InnerKp, motor1InnerKi, motor1InnerKd);
    tacho2.setPIDTunings(motor2InnerKp, motor2InnerKi, motor2InnerKd);
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
        if (slice.mode == CLOSED_LOOP_POSITION)
        {
            servo1.stop(); // Stop servo 1
            servo1.stop(); // Stop servo 2
        }
        else if (slice.mode == CLOSED_LOOP_SPEED)
        {
            tacho1.stop(); // Stop tacho 1
            tacho2.stop(); // Stop tacho 2
        }

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
    else if (!slice.eStop && slice.mode == CLOSED_LOOP_POSITION) // Process closed loop position
    {
        // Update the position setpoints for the motors
        servo1.moveTo(slice.motor1PositionSetpoint);
        servo2.moveTo(slice.motor2PositionSetpoint);       // need if statement to check change instead!!!!
        servo1.run();                                      // Run the servo 1 control loop
        servo2.run();                                      // Run the servo 2 control loop
        slice.motor1Position = servo1.getActualPosition(); // Update the current position from servo 1
        slice.motor2Position = servo2.getActualPosition(); // Update the current position from servo 2
    }
    else if (!slice.eStop && slice.mode == CLOSED_LOOP_SPEED) // Process closed loop speed
    {
        // Update the speed setpoints for the motors
        tacho1.setSpeedRPM(slice.motor1SpeedSetpoint);
        tacho2.setSpeedRPM(slice.motor2SpeedSetpoint);
        tacho1.run();                                     // Run the tacho 1 control loop
        tacho2.run();                                     // Run the tacho 2 control loop
        slice.motor1Speed = tacho1.getMeasuredSpeedRPM(); // Update the current speed from tacho 1
        slice.motor2Speed = tacho2.getMeasuredSpeedRPM(); // Update the current speed from tacho 2
    }
    else // unknown mode
    {
        // soft stop by setting brakes
        slice.motor1Brake = true;
        slice.motor2Brake = true;
        SLICE_DEBUG_PRINTLN(F("ERROR INVALID OPERATION MODE, ENTERED ERROR OR UNKNOWN STATE!"));
    }
}
