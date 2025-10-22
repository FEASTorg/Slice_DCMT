// src/serialCommands.cpp
#include <Arduino.h>
#include "globals.h"

void serialCommands()
{
    if (!Serial.available())
        return;

    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.startsWith("MODE="))
    {
        Serial.println(F("Setting mode unavailable in this firmware variant"));
    }
    else if (command.startsWith("M1PWM="))
    {
        if (slice.mode == OPEN_LOOP)
        {
            float v = command.substring(6).toFloat();
            if (v < -255)
                v = -255;
            if (v > 255)
                v = 255;
            slice.motor1PWM = v;
            Serial.print(F("M1PWM-> "));
            Serial.println(slice.motor1PWM);
        }
        else
        {
            Serial.println(F("Motor1 PWM can only be set in OPEN_LOOP mode"));
        }
    }
    else if (command.startsWith("M2PWM="))
    {
        if (slice.mode == OPEN_LOOP)
        {
            float v = command.substring(6).toFloat();
            if (v < -255)
                v = -255;
            if (v > 255)
                v = 255;
            slice.motor2PWM = v;
            Serial.print(F("M2PWM-> "));
            Serial.println(slice.motor2PWM);
        }
        else
        {
            Serial.println(F("Motor2 PWM can only be set in OPEN_LOOP mode"));
        }
    }
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
        Serial.print(F("Mode: OPEN_LOOP"));
        Serial.print(F(", M1PWM:"));
        Serial.print(slice.motor1PWM);
        Serial.print(F(", M2PWM:"));
        Serial.print(slice.motor2PWM);
        Serial.print(F(", B1:"));
        Serial.print(slice.motor1Brake);
        Serial.print(F(", B2:"));
        Serial.print(slice.motor2Brake);
        Serial.print(F(", ESTOP:"));
        Serial.println(slice.eStop);
    }
    else
    {
        Serial.println(F("Invalid command! Options: M1PWM=, M2PWM=, BRAKE1=0/1, BRAKE2=0/1, READ"));
    }
}
