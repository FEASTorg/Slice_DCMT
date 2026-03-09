#include <Wire.h>
#include <LMD18200.h>

// --- I2C Configuration ---
#define I2C_ADDRESS 14

// --- Motor 1 Pin Definitions ---
#define MOTOR1_DIR_PIN 7
#define MOTOR1_PWM_PIN 6
#define MOTOR1_BRAKE_PIN 8
#define MOTOR1_THERMAL_PIN 9
#define MOTOR1_CSENSE_PIN A6

// --- Motor 2 Pin Definitions ---
#define MOTOR2_DIR_PIN 10
#define MOTOR2_PWM_PIN 11
#define MOTOR2_BRAKE_PIN 12
#define MOTOR2_THERMAL_PIN 13
#define MOTOR2_CSENSE_PIN A7

// --- Global Constants ---
const float CURRENT_SENSE_RESISTOR = 4420.0; // Ohm value for current sense resistor
#define SERIAL_UPDATE_INTERVAL 1000          // ms

// --- Motor Driver Objects ---
LMD18200 motor1(MOTOR1_PWM_PIN, MOTOR1_DIR_PIN, MOTOR1_BRAKE_PIN, MOTOR1_CSENSE_PIN, MOTOR1_THERMAL_PIN);
LMD18200 motor2(MOTOR2_PWM_PIN, MOTOR2_DIR_PIN, MOTOR2_BRAKE_PIN, MOTOR2_CSENSE_PIN, MOTOR2_THERMAL_PIN);

// --- Command and State Variables ---
int8_t desiredSpeed = 0; // Commanded speed (-100 to 100)
int8_t motor1CommandSpeed = 0;
int8_t motor2CommandSpeed = 0;
byte selectedMotor = 0; // 1 for motor1, 2 for motor2
bool commandPending = false;
bool emergencyStop = false;

enum OperationMode
{
    CONTROL,
    WRITE
};
OperationMode currentMode = CONTROL;

unsigned long lastStatusPrintTime = 0;

// --- I2C Communication Buffer ---
byte i2cCommandBuffer[3];

// --- Function Prototypes ---
void processSerialCommand();
void updateMotorParameters();
void outputStatusCSV();
void checkEmergencyStop();
void handleI2CRequest();
void handleI2CReceive(int numBytes);

void setup()
{
    // Initialize motor drivers
    motor1.begin();
    motor2.begin();

    // Initialize Serial and I2C
    Serial.begin(115200);
    Wire.begin(I2C_ADDRESS);
    Wire.onReceive(handleI2CReceive);
    Wire.onRequest(handleI2CRequest);
}

void loop()
{
    processSerialCommand();

    // If a new command was received, update the appropriate motor
    if (commandPending)
    {
        updateMotorParameters();
        commandPending = false;
    }

    // Check both motors for thermal warnings; trigger estop if needed.
    checkEmergencyStop();

    // Output CSV status every SERIAL_UPDATE_INTERVAL milliseconds
    outputStatusCSV();
}

// --- Process Serial Commands ---
// Accepts commands like:
//   "M,0"         to switch to CONTROL mode,
//   "M,1"         to switch to WRITE mode,
//   "W,1,50"      to set motor1 speed to +50 (or negative for reverse),
//   "stop"        to trigger an emergency stop.
void processSerialCommand()
{
    if (Serial.available())
    {
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command.equalsIgnoreCase("stop"))
        {
            Serial.println("ESTOP activated. Shutting down motors.");
            motor1.brake();
            motor2.brake();
            emergencyStop = true;
            while (true)
            {
                delay(1000);
            }
        }
        else if (command.startsWith("M,"))
        {
            int modeValue = command.substring(2).toInt();
            currentMode = (modeValue == 0) ? CONTROL : WRITE;
            Serial.print("Switched to ");
            Serial.println((currentMode == CONTROL) ? "CONTROL mode" : "WRITE mode");
        }
        else if (command.startsWith("W,") && currentMode == WRITE)
        {
            int firstComma = command.indexOf(',');
            int secondComma = command.indexOf(',', firstComma + 1);
            if (firstComma != -1 && secondComma != -1)
            {
                selectedMotor = command.substring(firstComma + 1, secondComma).toInt();
                desiredSpeed = command.substring(secondComma + 1).toInt();
                // Enforce valid range [-100, 100]
                if (desiredSpeed < -100)
                    desiredSpeed = -100;
                if (desiredSpeed > 100)
                    desiredSpeed = 100;
                commandPending = true;
                Serial.print("Received command: Motor ");
                Serial.print(selectedMotor);
                Serial.print(", Speed ");
                Serial.println(desiredSpeed);
            }
            else
            {
                Serial.println("Invalid command format for motor update.");
            }
        }
        else
        {
            Serial.println("Invalid serial command!");
        }
    }
}

// --- Update Motor Parameters ---
// Maps the desired speed (-100 to 100) to PWM (0 to 255) and updates motor state.
void updateMotorParameters()
{
    int pwmValue = map(abs(desiredSpeed), 0, 100, 0, 255);

    if (selectedMotor == 1)
    {
        if (desiredSpeed == 0)
        {
            motor1.brake();
            motor1CommandSpeed = 0;
            Serial.println("Motor 1 braking.");
        }
        else
        {
            if (desiredSpeed < 0)
            {
                motor1.setSpeed(-pwmValue);
                motor1CommandSpeed = -desiredSpeed; // You may store signed value if needed.
                Serial.print("Motor 1 reverse, PWM:");
                Serial.println(pwmValue);
            }
            else
            {
                motor1.setSpeed(pwmValue);
                motor1CommandSpeed = desiredSpeed;
                Serial.print("Motor 1 forward, PWM:");
                Serial.println(pwmValue);
            }
        }
    }
    else if (selectedMotor == 2)
    {
        if (desiredSpeed == 0)
        {
            motor2.brake();
            motor2CommandSpeed = 0;
            Serial.println("Motor 2 braking.");
        }
        else
        {
            if (desiredSpeed < 0)
            {
                motor2.setSpeed(-pwmValue);
                motor2CommandSpeed = -desiredSpeed;
                Serial.print("Motor 2 reverse, PWM:");
                Serial.println(pwmValue);
            }
            else
            {
                motor2.setSpeed(pwmValue);
                motor2CommandSpeed = desiredSpeed;
                Serial.print("Motor 2 forward, PWM:");
                Serial.println(pwmValue);
            }
        }
    }
    else
    {
        Serial.println("Invalid motor selection.");
    }
}

// --- Check Emergency Stop Condition ---
// If either motor's thermal flag is active, trigger estop.
void checkEmergencyStop()
{
    if (motor1.isThermalWarning() || motor2.isThermalWarning())
    {
        emergencyStop = true;
        motor1.brake();
        motor2.brake();
        Serial.print("ESTOP triggered due to thermal warning! ");
        Serial.print("Triggered by Motor: ");
        Serial.println(motor1.isThermalWarning() ? "1" : "2");
        while (true)
        {
            delay(1000);
        }
    }
}

// --- Output Motor Status as CSV ---
// Prints a CSV-formatted status line with current time, mode, motor speeds, current readings, thermal flags, and estop status.
void outputStatusCSV()
{
    unsigned long currentMillis = millis();
    if (currentMillis - lastStatusPrintTime >= SERIAL_UPDATE_INTERVAL)
    {
        float m1Current = motor1.readCurrent(CURRENT_SENSE_RESISTOR);
        float m2Current = motor2.readCurrent(CURRENT_SENSE_RESISTOR);
        String m1Thermal = motor1.isThermalWarning() ? "active" : "normal";
        String m2Thermal = motor2.isThermalWarning() ? "active" : "normal";

        Serial.print("Mode:");
        Serial.print((currentMode == CONTROL) ? "CONTROL" : "WRITE");
        Serial.print(", Motor1Speed:");
        Serial.print(motor1CommandSpeed);
        Serial.print(", Motor1Current:");
        Serial.print(m1Current, 3);
        Serial.print(", Motor1Thermal:");
        Serial.print(m1Thermal);
        Serial.print(", Motor2Speed:");
        Serial.print(motor2CommandSpeed);
        Serial.print(", Motor2Current:");
        Serial.print(m2Current, 3);
        Serial.print(", Motor2Thermal:");
        Serial.print(m2Thermal);
        Serial.print(", ESTOP:");
        Serial.println(emergencyStop ? "true" : "false");

        lastStatusPrintTime = currentMillis;
    }
}

// --- I2C Event Handlers ---
void handleI2CRequest()
{
    // For simplicity, send motor speeds as two bytes.
    Serial.print("I2C Request: Motor1Speed:");
    Serial.print(motor1CommandSpeed);
    Serial.print(", Motor2Speed:");
    Serial.println(motor2CommandSpeed);

    Wire.write(motor1CommandSpeed);
    Wire.write(motor2CommandSpeed);
}

void handleI2CReceive(int numBytes)
{
    int i = 0;
    while (Wire.available())
    {
        i2cCommandBuffer[i] = Wire.read();
        i++;
    }
    // Expect at least 2 bytes: motorSelect and desiredSpeed.
    if (i >= 2)
    {
        selectedMotor = i2cCommandBuffer[0];
        desiredSpeed = i2cCommandBuffer[1];
        commandPending = true;
    }
}
