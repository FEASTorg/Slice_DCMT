#include <Arduino.h>

void serialCommands()
{
    if (Serial.available() > 0)
    {
        String command = Serial.readStringUntil('\n');
        command.trim();

        // Set operating mode.
        if (command.startsWith("MODE="))
        {
            Serial.println("Setting mode unavailable in this version");
        }
        // Set motor1 PWM value. Only available in OPEN_LOOP mode.
        else if (command.startsWith("M1PWM="))
        {
            if (slice.mode == OPEN_LOOP)
            {
                float value = command.substring(6).toFloat();
                if (value < -255)
                    value = -255;
                if (value > 255)
                    value = 255;
                slice.motor1PWM = value;
            }
            else
            {
                Serial.println("Motor1 PWM can only be set in OPEN_LOOP mode");
            }
        }
        // Set motor2 PWM value. Only available in OPEN_LOOP mode.
        else if (command.startsWith("M2PWM="))
        {
            if (slice.mode == OPEN_LOOP)
            {
                float value = command.substring(6).toFloat();
                if (value < -255)
                    value = -255;
                if (value > 255)
                    value = 255;
                slice.motor2PWM = value;
            }
            else
            {
                Serial.println("Motor2 PWM can only be set in OPEN_LOOP mode");
            }
        }
        else
        {
            Serial.println("Invalid command format!");
        }
    }
}