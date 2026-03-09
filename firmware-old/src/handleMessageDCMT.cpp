// src/handleMessageDCMT.cpp
#include <CRUMBS.h>
#include "globals.h"
#include "config.h"

void handleMessage(CRUMBSMessage &message)
{
    SLICE_DEBUG_PRINTLN(F("Slice: Received Message:"));
    SLICE_DEBUG_PRINT(F("typeID: "));
    SLICE_DEBUG_PRINTLN(message.typeID);
    SLICE_DEBUG_PRINT(F("commandType: "));
    SLICE_DEBUG_PRINTLN(message.commandType);
    SLICE_DEBUG_PRINT(F("data: "));
    for (int i = 0; i < 6; i++)
    {
        SLICE_DEBUG_PRINT(message.data[i]);
        SLICE_DEBUG_PRINT(' ');
    }
    SLICE_DEBUG_PRINTLN();
    SLICE_DEBUG_PRINT(F("errorFlags: "));
    SLICE_DEBUG_PRINTLN(message.errorFlags);

    if (message.typeID != TYPE_ID)
    {
        SLICE_DEBUG_PRINTLN(F("Slice: Type ID mismatch."));
        return;
    }

    switch (message.commandType)
    {
    case 0:
        SLICE_DEBUG_PRINTLN(F("Slice: Data Request Format Command Received."));
        break;

    case 1:
        SLICE_DEBUG_PRINTLN(F("Slice: Change Control Mode Command; NOT AVAILABLE IN THIS FIRMWARE VARIANT."));
        break;

    case 2:
        SLICE_DEBUG_PRINTLN(F("Slice: Setpoint Command Unavailable in this firmware variant."));
        break;

    case 3:
        SLICE_DEBUG_PRINTLN(F("Slice: PID Tuning Command Unavailable in this firmware variant."));
        break;

    case 4: // Brake Command
        slice.motor1Brake = (message.data[0] == 1);
        SLICE_DEBUG_PRINTLN(slice.motor1Brake ? F("Slice: Motor 1 Brake Engaged.") : F("Slice: Motor 1 Brake Released."));
        slice.motor2Brake = (message.data[1] == 1);
        SLICE_DEBUG_PRINTLN(slice.motor2Brake ? F("Slice: Motor 2 Brake Engaged.") : F("Slice: Motor 2 Brake Released."));
        break;

    case 6: // Open-loop write
        if (slice.mode == OPEN_LOOP)
        {
            double in1 = message.data[0];
            double in2 = message.data[1];
            in1 = (in1 < -255) ? -255 : (in1 > 255) ? 255
                                                    : in1;
            in2 = (in2 < -255) ? -255 : (in2 > 255) ? 255
                                                    : in2;
            slice.motor1PWM = (int)in1;
            slice.motor2PWM = (int)in2;
            SLICE_DEBUG_PRINT(F("Slice: Motor 1 input updated to: "));
            SLICE_DEBUG_PRINT(slice.motor1PWM);
            SLICE_DEBUG_PRINT(F(", Motor 2 input updated to: "));
            SLICE_DEBUG_PRINTLN(slice.motor2PWM);
        }
        else
        {
            SLICE_DEBUG_PRINTLN(F("Slice: ERROR: Not in OPEN_LOOP mode!"));
        }
        break;

    default:
        SLICE_DEBUG_PRINTLN(F("Slice: Unknown Command Type."));
        break;
    }

    SLICE_DEBUG_PRINTLN(F("Slice: Message processing complete."));
}
