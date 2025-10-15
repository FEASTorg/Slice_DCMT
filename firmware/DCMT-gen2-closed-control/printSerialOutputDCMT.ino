
/**
 * @def SLICE_DEBUG_PRINT
 * @brief Uncomment to enable print messages.
 */
#include "config.h"

void printSerialOutput()
{
    if (millis() - timing.lastSerialPrint >= SERIAL_UPDATE_TIME_MS) // change timing in config.h
    {
        SLICE_DEBUG_PRINT(F("ControlModes: ")); 
        if(slice.mode == CLOSED_LOOP_POSITION)
        {
            SLICE_DEBUG_PRINT(F("CLOSED_LOOP_POSITION"));
            SLICE_DEBUG_PRINT(F(", Motor1PositionSetpoint:"));
            SLICE_DEBUG_PRINT(slice.motor1PositionSetpoint);
            SLICE_DEBUG_PRINT(F(", Motor2PositionSetpoint:"));
            SLICE_DEBUG_PRINT(slice.motor2PositionSetpoint);
            SLICE_DEBUG_PRINT(F(", Motor1Position:"));
            SLICE_DEBUG_PRINT(slice.motor1Position);
            SLICE_DEBUG_PRINT(F(", Motor2Position:"));
            SLICE_DEBUG_PRINT(slice.motor2Position);
        }
        else if(slice.mode == CLOSED_LOOP_SPEED)
        {
            SLICE_DEBUG_PRINT(F("CLOSED_LOOP_SPEED"));
            SLICE_DEBUG_PRINT(F(", Motor1SpeedSetpoint:"));
            SLICE_DEBUG_PRINT(slice.motor1SpeedSetpoint);
            SLICE_DEBUG_PRINT(F(", Motor2SpeedSetpoint:"));
            SLICE_DEBUG_PRINT(slice.motor2SpeedSetpoint);
            SLICE_DEBUG_PRINT(F(", Motor1Speed:"));
            SLICE_DEBUG_PRINT(slice.motor1Speed);
            SLICE_DEBUG_PRINT(F(", Motor2Speed:"));
            SLICE_DEBUG_PRINT(slice.motor2Speed);
            SLICE_DEBUG_PRINT(F(", Motor1PWM:"));
            
        }
        else if(slice.mode == OPEN_LOOP)
        {
            SLICE_DEBUG_PRINT(F("OPEN_LOOP"));
            SLICE_DEBUG_PRINT(F(", Motor1PWM:"));
            SLICE_DEBUG_PRINT(slice.motor1PWM);
            SLICE_DEBUG_PRINT(F(", Motor2PWM:"));
            SLICE_DEBUG_PRINT(slice.motor2PWM);
        }
        
        SLICE_DEBUG_PRINT(F(", ESTOP:"));
        SLICE_DEBUG_PRINTLN(slice.eStop);

        timing.lastSerialPrint = millis();
    }
}
