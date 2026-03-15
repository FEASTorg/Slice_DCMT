// src/serialCommands.cpp
#include <Arduino.h>
#include "globals.h"

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

    if (command.startsWith("MODE="))
    {
        String mode = command.substring(5);
        mode.trim();
        mode.toUpperCase();

        if (mode == "OPEN")
            slice.mode = OPEN_LOOP;
        else if (mode == "POS")
            slice.mode = CLOSED_LOOP_POSITION;
#if DCMT_ENABLE_SPEED_LOOP
        else if (mode == "SPEED")
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
    else if (command.startsWith("M1PWM="))
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
    else if (command.startsWith("M2PWM="))
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
    else if (command.startsWith("M1POS="))
    {
        slice.motor1PositionSetpoint = (int16_t)command.substring(6).toInt();
        Serial.print(F("M1POS-> "));
        Serial.println(slice.motor1PositionSetpoint);
    }
    else if (command.startsWith("M2POS="))
    {
        slice.motor2PositionSetpoint = (int16_t)command.substring(6).toInt();
        Serial.print(F("M2POS-> "));
        Serial.println(slice.motor2PositionSetpoint);
    }
    else if (command.startsWith("PIDPOS="))
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
    else if (command.startsWith("M1SPEED="))
    {
        slice.motor1SpeedSetpoint = (int16_t)command.substring(8).toInt();
        Serial.print(F("M1SPEED-> "));
        Serial.println(slice.motor1SpeedSetpoint);
    }
    else if (command.startsWith("M2SPEED="))
    {
        slice.motor2SpeedSetpoint = (int16_t)command.substring(8).toInt();
        Serial.print(F("M2SPEED-> "));
        Serial.println(slice.motor2SpeedSetpoint);
    }
    else if (command.startsWith("PIDSPEED="))
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
    else if (command == "BRAKE1=1")
    {
        slice.motor1Brake = true;
        Serial.println(F("Motor1 brake engaged"));
    }
    else if (command == "BRAKE1=0")
    {
        slice.motor1Brake = false;
        Serial.println(F("Motor1 brake released"));
    }
    else if (command == "BRAKE2=1")
    {
        slice.motor2Brake = true;
        Serial.println(F("Motor2 brake engaged"));
    }
    else if (command == "BRAKE2=0")
    {
        slice.motor2Brake = false;
        Serial.println(F("Motor2 brake released"));
    }
    else if (command == "READ")
    {
        Serial.print(F("Mode:"));
        if (slice.mode == OPEN_LOOP)
            Serial.print(F("OPEN"));
        else if (slice.mode == CLOSED_LOOP_POSITION)
            Serial.print(F("POS"));
#if DCMT_ENABLE_SPEED_LOOP
        else
            Serial.print(F("SPEED"));
#else
        else
            Serial.print(F("UNKNOWN"));
#endif

        Serial.print(F(", M1PWM:"));
        Serial.print(slice.motor1PWM);
        Serial.print(F(", M2PWM:"));
        Serial.print(slice.motor2PWM);
        Serial.print(F(", M1POS_SP:"));
        Serial.print(slice.motor1PositionSetpoint);
        Serial.print(F(", M2POS_SP:"));
        Serial.print(slice.motor2PositionSetpoint);
        Serial.print(F(", M1POS:"));
        Serial.print(slice.motor1Position);
        Serial.print(F(", M2POS:"));
        Serial.print(slice.motor2Position);
#if DCMT_ENABLE_SPEED_LOOP
        Serial.print(F(", M1SPD_SP:"));
        Serial.print(slice.motor1SpeedSetpoint);
        Serial.print(F(", M2SPD_SP:"));
        Serial.print(slice.motor2SpeedSetpoint);
        Serial.print(F(", M1SPD:"));
        Serial.print(slice.motor1Speed);
        Serial.print(F(", M2SPD:"));
        Serial.print(slice.motor2Speed);
#endif
        Serial.print(F(", B1:"));
        Serial.print(slice.motor1Brake);
        Serial.print(F(", B2:"));
        Serial.print(slice.motor2Brake);
        Serial.print(F(", ESTOP:"));
        Serial.println(slice.eStop);
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
