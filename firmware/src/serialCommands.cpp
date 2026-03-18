// src/serialCommands.cpp
#include <Arduino.h>
#include <avr/pgmspace.h>
#include "globals.h"

static bool starts_with_p(const String &s, PGM_P prefix)
{
    size_t n = strlen_P(prefix);
    if (s.length() < (int)n)
        return false;
    return strncmp_P(s.c_str(), prefix, n) == 0;
}

static bool equals_p(const String &s, PGM_P token)
{
    return strcmp_P(s.c_str(), token) == 0;
}

static bool parse_pid_triplet(const String &s, float &kp, float &ki, float &kd)
{
    const int c1 = s.indexOf(',');
    const int c2 = s.indexOf(',', c1 + 1);
    if (c1 < 0 || c2 < 0)
        return false;

    kp = s.substring(0, c1).toFloat();
    ki = s.substring(c1 + 1, c2).toFloat();
    kd = s.substring(c2 + 1).toFloat();
    return true;
}

void serialCommands()
{
    if (!Serial.available())
        return;

    String command = Serial.readStringUntil('\n');
    command.trim();

    if (starts_with_p(command, PSTR("MODE=")))
    {
        String mode = command.substring(5);
        mode.trim();
        mode.toUpperCase();

        if (equals_p(mode, PSTR("OPEN")))
            slice.mode = OPEN_LOOP;
        else if (equals_p(mode, PSTR("POS")))
            slice.mode = CLOSED_LOOP_POSITION;
#if DCMT_ENABLE_SPEED_LOOP
        else if (equals_p(mode, PSTR("SPEED")))
            slice.mode = CLOSED_LOOP_SPEED;
#endif
        else
        {
#if DCMT_ENABLE_SPEED_LOOP
            Serial.println(F("MODE options: OPEN | POS | SPEED"));
#else
            Serial.println(F("MODE options: OPEN | POS"));
#endif
            return;
        }

        Serial.print(F("Mode-> "));
        if (slice.mode == OPEN_LOOP)
            Serial.println(F("OPEN_LOOP"));
        else if (slice.mode == CLOSED_LOOP_POSITION)
            Serial.println(F("CLOSED_LOOP_POSITION"));
#if DCMT_ENABLE_SPEED_LOOP
        else
            Serial.println(F("CLOSED_LOOP_SPEED"));
#endif
    }
    else if (starts_with_p(command, PSTR("M1PWM=")))
    {
        if (slice.mode != OPEN_LOOP)
        {
            Serial.println(F("M1PWM only valid in OPEN mode"));
            return;
        }

        long v = command.substring(6).toInt();
        if (v < -255)
            v = -255;
        if (v > 255)
            v = 255;
        slice.motor1PWM = (int16_t)v;
        Serial.print(F("M1PWM-> "));
        Serial.println(slice.motor1PWM);
    }
    else if (starts_with_p(command, PSTR("M2PWM=")))
    {
        if (slice.mode != OPEN_LOOP)
        {
            Serial.println(F("M2PWM only valid in OPEN mode"));
            return;
        }

        long v = command.substring(6).toInt();
        if (v < -255)
            v = -255;
        if (v > 255)
            v = 255;
        slice.motor2PWM = (int16_t)v;
        Serial.print(F("M2PWM-> "));
        Serial.println(slice.motor2PWM);
    }
    else if (starts_with_p(command, PSTR("M1POS=")))
    {
        slice.motor1PositionSetpoint = (int16_t)command.substring(6).toInt();
        Serial.print(F("M1POS-> "));
        Serial.println(slice.motor1PositionSetpoint);
    }
    else if (starts_with_p(command, PSTR("M2POS=")))
    {
        slice.motor2PositionSetpoint = (int16_t)command.substring(6).toInt();
        Serial.print(F("M2POS-> "));
        Serial.println(slice.motor2PositionSetpoint);
    }
    else if (starts_with_p(command, PSTR("PIDPOS=")))
    {
        float kp = 0.0f;
        float ki = 0.0f;
        float kd = 0.0f;
        if (!parse_pid_triplet(command.substring(7), kp, ki, kd))
        {
            Serial.println(F("PIDPOS format: PIDPOS=kp,ki,kd"));
            return;
        }
        slice.posPid1 = {kp, ki, kd};
        slice.posPid2 = {kp, ki, kd};
        Serial.println(F("Position PID updated"));
    }
#if DCMT_ENABLE_SPEED_LOOP
    else if (starts_with_p(command, PSTR("M1SPEED=")))
    {
        slice.motor1SpeedSetpoint = (int16_t)command.substring(8).toInt();
        Serial.print(F("M1SPEED-> "));
        Serial.println(slice.motor1SpeedSetpoint);
    }
    else if (starts_with_p(command, PSTR("M2SPEED=")))
    {
        slice.motor2SpeedSetpoint = (int16_t)command.substring(8).toInt();
        Serial.print(F("M2SPEED-> "));
        Serial.println(slice.motor2SpeedSetpoint);
    }
    else if (starts_with_p(command, PSTR("PIDSPEED=")))
    {
        float kp = 0.0f;
        float ki = 0.0f;
        float kd = 0.0f;
        if (!parse_pid_triplet(command.substring(9), kp, ki, kd))
        {
            Serial.println(F("PIDSPEED format: PIDSPEED=kp,ki,kd"));
            return;
        }
        slice.speedPid1 = {kp, ki, kd};
        slice.speedPid2 = {kp, ki, kd};
        Serial.println(F("Speed PID updated"));
    }
#endif
    else if (equals_p(command, PSTR("BRAKE1=1")))
    {
        slice.motor1Brake = true;
        Serial.println(F("Motor1 brake engaged"));
    }
    else if (equals_p(command, PSTR("BRAKE1=0")))
    {
        slice.motor1Brake = false;
        Serial.println(F("Motor1 brake released"));
    }
    else if (equals_p(command, PSTR("BRAKE2=1")))
    {
        slice.motor2Brake = true;
        Serial.println(F("Motor2 brake engaged"));
    }
    else if (equals_p(command, PSTR("BRAKE2=0")))
    {
        slice.motor2Brake = false;
        Serial.println(F("Motor2 brake released"));
    }
    else if (equals_p(command, PSTR("READ")))
    {
        printSliceState(Serial);
    }
    else
    {
        Serial.println(F("Invalid command."));
        Serial.println(F("Open/Pos: MODE=OPEN|POS, M1PWM=, M2PWM=, M1POS=, M2POS=, PIDPOS=kp,ki,kd, BRAKE1=0/1, BRAKE2=0/1, READ"));
#if DCMT_ENABLE_SPEED_LOOP
        Serial.println(F("Closed-loop: MODE=POS|SPEED, M1POS=, M2POS=, M1SPEED=, M2SPEED=, PIDPOS=kp,ki,kd, PIDSPEED=kp,ki,kd"));
#endif
    }
}
