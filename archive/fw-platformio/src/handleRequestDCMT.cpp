// src/handleRequestDCMT.cpp
#include <Wire.h>
#include <CRUMBS.h>
#include "globals.h"
#include "config.h"

void handleRequest()
{
    SLICE_DEBUG_PRINTLN(F("Slice: Controller requested data, sending response..."));

    CRUMBSMessage responseMessage;
    responseMessage.typeID = TYPE_ID;
    responseMessage.commandType = 0;

    bool brakeFlag = slice.motor1Brake || slice.motor2Brake;

    responseMessage.data[0] = slice.mode;
    responseMessage.data[1] = slice.motor1PWM;
    responseMessage.data[2] = slice.motor2PWM;
    responseMessage.data[3] = 0;
    responseMessage.data[4] = 0;
    responseMessage.data[5] = brakeFlag;

    responseMessage.errorFlags = 0;

    uint8_t buffer[CRUMBS_MESSAGE_SIZE];
    size_t encodedSize = crumbsSlice.encodeMessage(responseMessage, buffer, sizeof(buffer));
    if (encodedSize == 0)
    {
        SLICE_DEBUG_PRINTLN(F("Slice: Failed to encode response message."));
        return;
    }

    Wire.write(buffer, encodedSize);
    SLICE_DEBUG_PRINTLN(F("Slice: Response message sent."));
}
