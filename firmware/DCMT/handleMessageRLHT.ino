#include <CRUMBS.h>

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
        // CommandType 1: Change ControlMode Command; UNIVERSAL
        currentMode = (ControlMode)message.data[0]; // Cast data to ControlMode enum where 0 = CLOSED_LOOP, 1 = OPEN_LOOP
        SLICE_DEBUG_PRINT(F("Slice: ControlMode Change Command Received. ControlMode: "));
        SLICE_DEBUG_PRINTLN(currentMode == CLOSED_LOOP ? F("CLOSED_LOOP") : F("OPEN_LOOP"));
        break;

    case 2:
        // CommandType 2: Change Setpoint Command; SEMI-UNIVERSAL
        slice.relayHeater1.setpointTemperature = message.data[0]; // Set desired temperature for heater 1
        slice.relayHeater2.setpointTemperature = message.data[1]; // Set desired temperature for heater 2
        SLICE_DEBUG_PRINT(F("Slice: Setpoint Change Command Received. Setpoint1: "));
        SLICE_DEBUG_PRINT(slice.relayHeater1.setpointTemperature);
        SLICE_DEBUG_PRINT(F(", Setpoint2: "));
        SLICE_DEBUG_PRINTLN(slice.relayHeater2.setpointTemperature);
        break;

    case 3:
        // CommandType 3: Change PID Tuning Command
        slice.relayHeater1.Kp = message.data[0]; // Set PID proportional gain for heater 1
        slice.relayHeater1.Ki = message.data[1]; // Set PID integral gain for heater 1
        slice.relayHeater1.Kd = message.data[2]; // Set PID derivative gain for heater 1
        slice.relayHeater2.Kp = message.data[3]; // Set PID proportional gain for heater 2
        slice.relayHeater2.Ki = message.data[4]; // Set PID integral gain for heater 2
        slice.relayHeater2.Kd = message.data[5]; // Set PID derivative gain for heater 2
        SLICE_DEBUG_PRINT(F("Slice: PID Tuning Change Command Received. Kp1: "));
        SLICE_DEBUG_PRINT(slice.relayHeater1.Kp);
        SLICE_DEBUG_PRINT(F(", Ki1: "));
        SLICE_DEBUG_PRINT(slice.relayHeater1.Ki);
        SLICE_DEBUG_PRINT(F(", Kd1: "));
        SLICE_DEBUG_PRINT(slice.relayHeater1.Kd);
        SLICE_DEBUG_PRINT(F(", Kp2: "));
        SLICE_DEBUG_PRINT(slice.relayHeater2.Kp);
        SLICE_DEBUG_PRINT(F(", Ki2: "));
        SLICE_DEBUG_PRINT(slice.relayHeater2.Ki);
        SLICE_DEBUG_PRINT(F(", Kd2: "));
        SLICE_DEBUG_PRINTLN(slice.relayHeater2.Kd);
        break;

    case 4:
        // CommandType 4: Change Relay Period Command
        slice.relayHeater1.relayPeriod = message.data[0]; // Set duration of relay 1 cycle in ms
        slice.relayHeater2.relayPeriod = message.data[1]; // Set duration of relay 2 cycle in ms
        SLICE_DEBUG_PRINT(F("Slice: Relay Period Change Command Received. Period1: "));
        SLICE_DEBUG_PRINT(slice.relayHeater1.relayPeriod);
        SLICE_DEBUG_PRINT(F(", Period2: "));
        SLICE_DEBUG_PRINTLN(slice.relayHeater2.relayPeriod);
        break;

    case 5:
        // CommandType 5: Change Thermocouple Select Command
        slice.relayHeater1.thermocoupleSelect = message.data[0]; // Select which thermocouple pairs with relay 1
        slice.relayHeater2.thermocoupleSelect = message.data[1]; // Select which thermocouple pairs with relay 2
        SLICE_DEBUG_PRINT(F("Slice: Thermocouple Select Command Received. ThermoSelect1: "));
        SLICE_DEBUG_PRINT(slice.relayHeater1.thermocoupleSelect);
        SLICE_DEBUG_PRINT(F(", ThermoSelect2: "));
        SLICE_DEBUG_PRINTLN(slice.relayHeater2.thermocoupleSelect);
        break;

    case 6:
        // CommandType 6: Write to the relays directly (write mode only)
        if (currentMode == OPEN_LOOP)
        {
            // the expectation is that data0 is relay 1 and data1 is relay 2 and both are 0-100% duty cycle
            double input1 = message.data[0]; // Extract relay input
            double input2 = message.data[1]; // Extract relay input

            // limit it from 0 to 100
            input1 = (input1 < 0) ? 0 : (input1 > 100) ? 100
                                                       : input1;
            input2 = (input2 < 0) ? 0 : (input2 > 100) ? 100
                                                       : input2;
            // cast the input to an integer
            int newOnTime1 = (int)input1;
            int newOnTime2 = (int)input2;

            // scale from 0 to 100 to 0 to rPeriod
            newOnTime1 = map(newOnTime1, 0, 100, 0, slice.relayHeater1.relayPeriod);
            newOnTime2 = map(newOnTime2, 0, 100, 0, slice.relayHeater2.relayPeriod);

            // finally we update the actual relay on time values
            slice.relayHeater1.relayOnTime = newOnTime1;
            slice.relayHeater2.relayOnTime = newOnTime2;

            SLICE_DEBUG_PRINT(F("Slice: Relay 1 input updated to: "));
            SLICE_DEBUG_PRINT(input1);
            SLICE_DEBUG_PRINT(F(", Relay 2 input updated to: "));
            SLICE_DEBUG_PRINTLN(input2);
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