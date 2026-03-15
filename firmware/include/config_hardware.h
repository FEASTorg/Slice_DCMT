#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

#include "config.h"

// ----- General BREAD -----
#define ESTOP 2
#define LED_PIN 5

// ----- DCMT Specific -----

// Timing constants
#define SERIAL_UPDATE_TIME_MS 1000

// Proven closed-loop tuning defaults from archive implementation.
#define DCMT_POS_PID_KP 0.15f
#define DCMT_POS_PID_KI 0.01f
#define DCMT_POS_PID_KD 0.01f

#define DCMT_SPEED_PID_KP 0.15f
#define DCMT_SPEED_PID_KI 0.01f
#define DCMT_SPEED_PID_KD 0.01f

#define DCMT_SERVO_PWM_SKIP 15
#define DCMT_SERVO_MAX_PWM 150
#define DCMT_SERVO_ACCURACY 10

// Speed-loop update interval in milliseconds.
#define DCMT_TACHO_INTERVAL_MS 5

// ----- Motor1 Definitions -----
#if (DCMT_HW_GEN == 1)
#define MOTOR1_PWM_PIN 6
#define MOTOR1_DIR_PIN 5
#define MOTOR1_BRAKE_PIN 7
#elif (DCMT_HW_GEN == 2)
#define MOTOR1_PWM_PIN 6
#define MOTOR1_DIR_PIN 7
#define MOTOR1_BRAKE_PIN 8
#define MOTOR1_THERMAL_PIN 9
#else
#error "Unsupported DCMT_HW_GEN value"
#endif
#define MOTOR1_ENCODER_PIN1 A2
#define MOTOR1_ENCODER_PIN2 A3
#define MOTOR1_CSENSE_PIN A6

// ----- Motor2 Definitions -----
#if (DCMT_HW_GEN == 1)
#define MOTOR2_PWM_PIN 10
#define MOTOR2_DIR_PIN 9
#define MOTOR2_BRAKE_PIN 11
#elif (DCMT_HW_GEN == 2)
#define MOTOR2_PWM_PIN 11
#define MOTOR2_DIR_PIN 10
#define MOTOR2_BRAKE_PIN 12
#define MOTOR2_THERMAL_PIN 13
#else
#error "Unsupported DCMT_HW_GEN value"
#endif
#define MOTOR2_ENCODER_PIN1 A0
#define MOTOR2_ENCODER_PIN2 A1
#define MOTOR2_CSENSE_PIN A7

// Encoder counts-per-rev (archive-proven closed-loop value).
#define MOTOR1_CPR 798
#define MOTOR2_CPR 798

#endif // HARDWARE_CONFIG_H
