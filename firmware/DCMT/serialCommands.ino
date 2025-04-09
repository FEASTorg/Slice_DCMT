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
            String modeStr = command.substring(5);
            modeStr.trim();
            if (modeStr.equalsIgnoreCase("OPEN"))
            {
                slice.mode = OPEN_LOOP;
            }
            else if (modeStr.equalsIgnoreCase("POS"))
            {
                slice.mode = CLOSED_LOOP_POSITION;
            }
            else if (modeStr.equalsIgnoreCase("SPEED"))
            {
                slice.mode = CLOSED_LOOP_SPEED;
            }
            else
            {
            }
        }
        // Set motor1 PWM value. Only available in OPEN_LOOP mode.
        else if (command.startsWith("M1PWM="))
        {
            if (slice.mode == OPEN_LOOP)
            {
                float value = command.substring(6).toFloat();
                if (value < 0)
                    value = 0;
                if (value > 255)
                    value = 255;
                slice.motor1PWM = value;
            }
            else
            {
                //Serial.println("Motor1 PWM can only be set in OPEN_LOOP mode");
            }
        }
        // Set motor2 PWM value. Only available in OPEN_LOOP mode.
        else if (command.startsWith("M2PWM="))
        {
            if (slice.mode == OPEN_LOOP)
            {
                float value = command.substring(6).toFloat();
                if (value < 0)
                    value = 0;
                if (value > 255)
                    value = 255;
                slice.motor2PWM = value;
            }
            else
            {
                //Serial.println("Motor2 PWM can only be set in OPEN_LOOP mode");
            }
        }
        // Set motor1 position setpoint. Only available in CLOSED_LOOP_POSITION mode.
        else if (command.startsWith("M1POS="))
        {
            if (slice.mode == CLOSED_LOOP_POSITION)
            {
                float value = command.substring(6).toFloat();
                slice.motor1PositionSetpoint = value;
            }
            else
            {
                //Serial.println("Motor1 position can only be set in CLOSED_LOOP_POSITION mode");
            }
        }
        // Set motor2 position setpoint. Only available in CLOSED_LOOP_POSITION mode.
        else if (command.startsWith("M2POS="))
        {
            if (slice.mode == CLOSED_LOOP_POSITION)
            {
                float value = command.substring(6).toFloat();
                slice.motor2PositionSetpoint = value;
            }
            else
            {
                //Serial.println("Motor2 position can only be set in CLOSED_LOOP_POSITION mode");
            }
        }
        // Set motor1 speed setpoint. Only available in CLOSED_LOOP_SPEED mode.
        else if (command.startsWith("M1SPEED="))
        {
            if (slice.mode == CLOSED_LOOP_SPEED)
            {
                float value = command.substring(8).toFloat();
                slice.motor1SpeedSetpoint = value;
            }
            else
            {
                //Serial.println("Motor1 speed can only be set in CLOSED_LOOP_SPEED mode");
            }
        }
        // Set motor2 speed setpoint. Only available in CLOSED_LOOP_SPEED mode.
        else if (command.startsWith("M2SPEED="))
        {
            if (slice.mode == CLOSED_LOOP_SPEED)
            {
                float value = command.substring(8).toFloat();
                slice.motor2SpeedSetpoint = value;
            }
            else
            {
                //Serial.println("Motor2 speed can only be set in CLOSED_LOOP_SPEED mode");
            }
        }
        else
        {
            //Serial.println("Invalid command format!");
        }
    }
}