#include <CRUMBS.h>
#include "config.h"

/**
 * @brief Callback function to handle received CRUMBSMessages from the Controller.
 *
 * @param message The received CRUMBSMessage.
 */
void handleMessage(CRUMBSMessage &message)
{
    // Debug print the received message
    SLICE_DEBUG_PRINTLN(F("Slice: Received Message:"));
    SLICE_DEBUG_PRINT(F("typeID: "));
    SLICE_DEBUG_PRINTLN(message.typeID);
    SLICE_DEBUG_PRINT(F("commandType: "));
    SLICE_DEBUG_PRINTLN(message.commandType);
    SLICE_DEBUG_PRINT(F("data: "));
    for (int i = 0; i < 6; i++)
    {
        SLICE_DEBUG_PRINT(message.data[i]);
        SLICE_DEBUG_PRINT(F(" "));
    }
    SLICE_DEBUG_PRINTLN();
    SLICE_DEBUG_PRINT(F("errorFlags: "));
    SLICE_DEBUG_PRINTLN(message.errorFlags);

    // Check if typeID matches the expected value
    if (message.typeID != TYPE_ID)
    {
        SLICE_DEBUG_PRINTLN(F("Slice: Type ID mismatch."));
        return;
    }

    // Process the message based on commandType
    // structure of the message is as follows:
    switch (message.commandType)
    {
    case 0:
        // CommandType 0: Data Request Format Command; UNIVERSAL
        SLICE_DEBUG_PRINTLN(F("Slice: Data Request Format Command Received."));

        // implement later, the idea is that the leader can request the format of the data
        // that the follower will send back in response to a data request

        break;

    case 1:
        // CommandType 1: Change ControlModes Command; NOT AVAILABLE IN THIS FIRMWARE VARIANT
        SLICE_DEBUG_PRINTLN(F("Slice: Change Control Mode Command; NOT AVAILABLE IN THIS FIRMWARE VARIANT."));

    case 2:
        // CommandType 2: Change Setpoint Command; NOT AVAILABLE IN THIS FIRMWARE VARIANT
        SLICE_DEBUG_PRINTLN(F("Slice: Setpoint Command Unavaialble in this firmware variant."));

        break;

    case 3:
        // CommandType 3: Change PID Tuning Command; NOT AVAILABLE IN THIS FIRMWARE VARIANT
        SLICE_DEBUG_PRINTLN(F("Slice: PID Tuning Command Unavaialble in this firmware variant."));

    case 4:
        // CommandType 4: Brake Command
        if (message.data[0] == 1)
        {
            slice.motor1Brake = true; // Set motor 1 brake to engaged
            SLICE_DEBUG_PRINTLN(F("Slice: Motor 1 Brake Engaged."));
        }
        else
        {
            slice.motor1Brake = false; // Set motor 1 brake to released
            SLICE_DEBUG_PRINTLN(F("Slice: Motor 1 Brake Released."));
        }
        if (message.data[1] == 1)
        {
            slice.motor2Brake = true; // Set motor 2 brake to engaged
            SLICE_DEBUG_PRINTLN(F("Slice: Motor 2 Brake Engaged."));
        }
        else
        {
            slice.motor2Brake = false; // Set motor 2 brake to released
            SLICE_DEBUG_PRINTLN(F("Slice: Motor 2 Brake Released."));
        }

    case 6:
        // CommandType 6: Write to the motors directly (write mode only), can probably wrap into the setpoint command
        if (slice.mode == OPEN_LOOP)
        {
            // the expectation is that data0 is relay 1 and data1 is relay 2 and both are 0-100% duty cycle
            double input1 = message.data[0]; // Extract relay input
            double input2 = message.data[1]; // Extract relay input

            // limit it from 0 to 255
            input1 = (input1 < 0) ? 0 : (input1 > 255) ? 255
                                                       : input1;
            input2 = (input2 < 0) ? 0 : (input2 > 255) ? 255
                                                       : input2;
            // cast the input to an integer
            int newPWM1 = (int)input1;
            int newPWM2 = (int)input2;

            // finally we update the actual motor pwm values
            slice.motor1PWM = newPWM1; // Update motor 1 PWM value
            slice.motor2PWM = newPWM2; // Update motor 2 PWM value

            SLICE_DEBUG_PRINT(F("Slice: Motor 1 input updated to: "));
            SLICE_DEBUG_PRINT(newPWM1);
            SLICE_DEBUG_PRINT(F(", Motor 2 input updated to: "));
            SLICE_DEBUG_PRINTLN(newPWM2);
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