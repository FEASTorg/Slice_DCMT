
/**
 * @def SLICE_PRINT
 * @brief Uncomment to enable print messages.
 */

#define SLICE_PRINT_SERIAL 1

#ifdef SLICE_PRINT_SERIAL

#define SLICE_PRINT(...) \
    Serial.print(__VA_ARGS__)

#define SLICE_PRINTLN(...) \
    Serial.println(__VA_ARGS__)

#else
#define SLICE_PRINT(...)
#define SLICE_PRINTLN(...)
#endif

void printSerialOutput()
{
    if (millis() - timing.lastSerialPrint >= SERIAL_UPDATE_TIME_MS) // change timing in config.h
    {
        SLICE_PRINT(F("ControlMode: "));
        SLICE_PRINT(currentMode == CLOSED_LOOP ? "CLOSED_LOOP" : "OPEN_LOOP");
        SLICE_PRINT(F(", T1:"));
        SLICE_PRINT(slice.temperature1);
        SLICE_PRINT(F(", T2:"));
        SLICE_PRINT(slice.temperature2);

        SLICE_PRINT(F(", PID1:"));
        SLICE_PRINT(slice.relayHeater1.Kp);
        SLICE_PRINT(F(","));
        SLICE_PRINT(slice.relayHeater1.Ki);
        SLICE_PRINT(F(","));
        SLICE_PRINT(slice.relayHeater1.Kd);

        SLICE_PRINT(F(", Relay1Input:"));
        SLICE_PRINT(slice.relayHeater1.inputTemperature);
        SLICE_PRINT(F(", Setpoint1:"));
        SLICE_PRINT(slice.relayHeater1.setpointTemperature);
        SLICE_PRINT(F(", onTime1:"));
        SLICE_PRINT((int)slice.relayHeater1.relayOnTime);
        SLICE_PRINT(F(", rPeriod1:"));
        SLICE_PRINT(slice.relayHeater1.relayPeriod);

        SLICE_PRINT(F(", PID2:"));
        SLICE_PRINT(slice.relayHeater2.Kp);
        SLICE_PRINT(F(","));
        SLICE_PRINT(slice.relayHeater2.Ki);
        SLICE_PRINT(F(","));
        SLICE_PRINT(slice.relayHeater2.Kd);

        SLICE_PRINT(F(", Relay2Input:"));
        SLICE_PRINT(slice.relayHeater2.inputTemperature);
        SLICE_PRINT(F(", Setpoint2:"));
        SLICE_PRINT(slice.relayHeater2.setpointTemperature);
        SLICE_PRINT(F(", onTime2:"));
        SLICE_PRINT((int)slice.relayHeater2.relayOnTime);
        SLICE_PRINT(F(", rPeriod2:"));
        SLICE_PRINT(slice.relayHeater2.relayPeriod);
        SLICE_PRINT(F(", Thermo Select Relay 1:"));
        SLICE_PRINT(slice.relayHeater1.thermocoupleSelect);
        SLICE_PRINT(F(", Thermo Select Relay 2:"));
        SLICE_PRINT(slice.relayHeater2.thermocoupleSelect);
        SLICE_PRINT(F(", ESTOP:"));
        SLICE_PRINTLN(slice.eStop);

        timing.lastSerialPrint = millis();
    }
}
