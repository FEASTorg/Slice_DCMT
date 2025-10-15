#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

// ----- General BREAD -----
#define ESTOP 2
#define LED_PIN 5

// ----- DCMT Specific -----

// Timing constants
#define SERIAL_UPDATE_TIME_MS 1000

// Control modes
enum ControlModes
{
    CLOSED_LOOP_POSITION,
    CLOSED_LOOP_SPEED,
    OPEN_LOOP
};

// ----- Motor1 Definitions -----
#define MOTOR1_PWM_PIN 6
#define MOTOR1_DIR_PIN 7
#define MOTOR1_BRAKE_PIN 8
#define MOTOR1_THERMAL_PIN 9
#define MOTOR1_ENCODER_PIN1 A2
#define MOTOR1_ENCODER_PIN2 A3
#define MOTOR1_CSENSE_PIN A6

// ----- Motor2 Definitions -----
#define MOTOR2_PWM_PIN 11
#define MOTOR2_DIR_PIN 10
#define MOTOR2_BRAKE_PIN 12
#define MOTOR2_THERMAL_PIN 13
#define MOTOR2_ENCODER_PIN1 A0
#define MOTOR2_ENCODER_PIN2 A1
#define MOTOR2_CSENSE_PIN A7

#endif // HARDWARE_CONFIG_H

#define DIR1 5
#define MC1 6
#define BR1 7
#define DIR2 9
#define MC2 10
#define BR2 11
