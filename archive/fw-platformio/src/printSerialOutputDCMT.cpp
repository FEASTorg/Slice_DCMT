#include "globals.h"
#include "config.h"

void printSerialOutput()
{
    if (millis() - timing.lastSerialPrint >= SERIAL_UPDATE_TIME_MS)
    {
        SLICE_DEBUG_PRINT(F(", Motor1PWM:"));
        SLICE_DEBUG_PRINT(slice.motor1PWM);
        SLICE_DEBUG_PRINT(F(", Motor2PWM:"));
        SLICE_DEBUG_PRINT(slice.motor2PWM);
        SLICE_DEBUG_PRINT(F(", ESTOP:"));
        SLICE_DEBUG_PRINTLN(slice.eStop);
        timing.lastSerialPrint = millis();
    }
}
