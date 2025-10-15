#include <CRUMBS.h>
#include "config.h"

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
    responseMessage.typeID = TYPE_ID;                   /**< SLICE type ID */
    responseMessage.commandType = 0;                    /**< CommandType 0 for status response */
    bool brakeFlag = slice.motor1Brake || slice.motor2Brake; /**< Combined brake status */

    responseMessage.data[0] = slice.mode;      /**< Mode of operation */
    responseMessage.data[1] = slice.motor1PWM; /**< Motor 1 PWM value */
    responseMessage.data[2] = slice.motor2PWM; /**< Motor 2 PWM value */
    responseMessage.data[3] = 0;               /**< Left Blank */
    responseMessage.data[4] = 0;               /**< Left Blank */
    responseMessage.data[5] = brakeFlag;       /**< Brake status */

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
