#include <CRUMBS.h>

/**
 * @brief Callback function to handle data requests from the Controller.
 *
 * @note This function sends a CRUMBSMessage back to the Controller in response to a request.
 */
void handleRequest()
{
    SLICE_DEBUG_PRINTLN(F("Slice: Controller requested data, sending response..."));

    // Prepare response message
    CRUMBSMessage responseMessage;
    responseMessage.typeID = 1;      /**< SLICE type ID */
    responseMessage.commandType = 0; /**< CommandType 0 for status response */

    // Populate data fields with example data
    responseMessage.data[0] = slice.temperature1;                     /**< Thermocouple 1 temperature */
    responseMessage.data[1] = slice.temperature2;                     /**< Thermocouple 2 temperature */
    responseMessage.data[2] = slice.relayHeater1.setpointTemperature; /**< Heater 1 setpoint */
    responseMessage.data[3] = slice.relayHeater2.setpointTemperature; /**< Heater 2 setpoint */
    responseMessage.data[4] = slice.relayHeater1.relayOnTime;         /**< Relay 1 on time */
    responseMessage.data[5] = slice.relayHeater2.relayOnTime;         /**< Relay 2 on time */

    responseMessage.errorFlags = 0; /**< No errors */

    uint8_t buffer[CRUMBS_MESSAGE_SIZE];
    size_t encodedSize = crumbsSlice.encodeMessage(responseMessage, buffer, sizeof(buffer));

    if (encodedSize == 0)
    {
        SLICE_DEBUG_PRINTLN(F("Slice: Failed to encode response message."));
        return;
    }

    // Send the encoded message back to the Controller
    Wire.write(buffer, encodedSize);
    SLICE_DEBUG_PRINTLN(F("Slice: Response message sent."));
}

