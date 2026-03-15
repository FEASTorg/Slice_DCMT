#include "globals.h"
#include "config.h"

void printSerialOutput()
{
    if (millis() - timing.lastSerialPrint >= SERIAL_UPDATE_TIME_MS)
    {
        SLICE_DEBUG_PRINT(F("MODE:"));
        if (slice.mode == OPEN_LOOP)
            SLICE_DEBUG_PRINT(F("OPEN"));
        else if (slice.mode == CLOSED_LOOP_POSITION)
            SLICE_DEBUG_PRINT(F("POS"));
#if DCMT_ENABLE_SPEED_LOOP
        else
            SLICE_DEBUG_PRINT(F("SPEED"));
#else
        else
            SLICE_DEBUG_PRINT(F("UNKNOWN"));
#endif

        SLICE_DEBUG_PRINT(F(", Motor1PWM:"));
        SLICE_DEBUG_PRINT(slice.motor1PWM);
        SLICE_DEBUG_PRINT(F(", Motor2PWM:"));
        SLICE_DEBUG_PRINT(slice.motor2PWM);
        SLICE_DEBUG_PRINT(F(", M1POS_SP:"));
        SLICE_DEBUG_PRINT(slice.motor1PositionSetpoint);
        SLICE_DEBUG_PRINT(F(", M2POS_SP:"));
        SLICE_DEBUG_PRINT(slice.motor2PositionSetpoint);
        SLICE_DEBUG_PRINT(F(", M1POS:"));
        SLICE_DEBUG_PRINT(slice.motor1Position);
        SLICE_DEBUG_PRINT(F(", M2POS:"));
        SLICE_DEBUG_PRINT(slice.motor2Position);
#if DCMT_ENABLE_SPEED_LOOP
        SLICE_DEBUG_PRINT(F(", M1SPD_SP:"));
        SLICE_DEBUG_PRINT(slice.motor1SpeedSetpoint);
        SLICE_DEBUG_PRINT(F(", M2SPD_SP:"));
        SLICE_DEBUG_PRINT(slice.motor2SpeedSetpoint);
        SLICE_DEBUG_PRINT(F(", M1SPD:"));
        SLICE_DEBUG_PRINT(slice.motor1Speed);
        SLICE_DEBUG_PRINT(F(", M2SPD:"));
        SLICE_DEBUG_PRINT(slice.motor2Speed);
#endif
        SLICE_DEBUG_PRINT(F(", ESTOP:"));
        SLICE_DEBUG_PRINTLN(slice.eStop);
        timing.lastSerialPrint = millis();
    }
}
