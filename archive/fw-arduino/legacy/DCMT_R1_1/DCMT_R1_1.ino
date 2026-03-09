#include <Wire.h>
// #include <SPI.h>

#define I2C_ADR 14 // Set this however you want

#define DIR1 7
#define MC1 6
#define BR1 8
// #define THRM1 9
// #define CSEN1 A6
#define DIR2 10
#define MC2 11
#define BR2 12
// #define THRM2 13
// #define CSEN2 A7

// #define B2 A0
// #define A2 A1
// #define B1 A2
// #define A1 A3

bool dir1 = 1; // start with forward
bool dir2 = 1;
bool br1 = 1; // start with brake on
bool br2 = 1;

int pwm1, pwm2 = 0; // start with 0 speed

typedef union // Define a float that can be broken up and sent via I2C
{
    float number;
    uint8_t bytes[4];
} FLOATUNION_t;

FLOATUNION_t RX;

byte commandSelect[3];
int8_t speedSelect;
int8_t motor1Speed;
int8_t motor2Speed;
byte motorSelect;
bool changeParam = false;

#define SERIAL_UPDATE_TIME_MS 1000

long lastSerialPrint = 0;

enum Mode
{
    CONTROL,
    WRITE
};

Mode currentMode = CONTROL;

void setup()
{
    pinMode(MC1, OUTPUT);
    pinMode(MC2, OUTPUT);
    pinMode(DIR1, OUTPUT);
    pinMode(DIR2, OUTPUT);
    pinMode(BR1, OUTPUT);
    pinMode(BR2, OUTPUT);

    Serial.begin(115200); // updated baud rate
    Wire.begin(I2C_ADR);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);
}

void loop()
{
    serialCommandHandler();

    if (changeParam)
    {
        switch (motorSelect)
        {
        case 1: // Motor 1 edit
            if (speedSelect == 0)
            {
                br1 = 1;
                pwm1 = 0;
                Serial.println("Motor 1 brake");
            }
            else if (speedSelect < 0 && speedSelect >= -100)
            {
                br1 = 0;
                dir1 = 0;
                speedSelect *= -1;
                pwm1 = map(speedSelect, 0, 100, 0, 255);
                Serial.print("Motor 1 back ");
                Serial.println(pwm1);
            }
            else if (speedSelect > 0 && speedSelect <= 100)
            {
                br1 = 0;
                dir1 = 1;
                pwm1 = map(speedSelect, 0, 100, 0, 255);
                Serial.print("Motor 1 forward ");
                Serial.println(pwm1);
            }
            else
            {
                Serial.println("Invalid Entry, nothing will be changed");
            }
            break;

        case 2: // motor 2 edit
            if (speedSelect == 0)
            {
                br2 = 1;
                pwm2 = 0;
                Serial.println("Motor 2 brake");
            }
            else if (speedSelect < 0 && speedSelect >= -100)
            {
                br2 = 0;
                dir2 = 0;
                speedSelect *= -1;
                pwm2 = map(speedSelect, 0, 100, 0, 255);
                Serial.print("Motor 2 back ");
                Serial.println(pwm2);
            }
            else if (speedSelect > 0 && speedSelect <= 100)
            {
                br2 = 0;
                dir2 = 1;
                pwm2 = map(speedSelect, 0, 100, 0, 255);
                Serial.print("Motor 2 forward ");
                Serial.println(pwm2);
            }
            else
            {
                Serial.println("Invalid Entry, nothing will be changed");
            }
            break;

        default:
            Serial.println("oopsie poopsie, that's not a command, silly");
            break;
        }
        changeParam = false;
    }

    digitalWrite(DIR1, dir1);
    analogWrite(MC1, pwm1);
    digitalWrite(BR1, br1);
    digitalWrite(DIR2, dir2);
    analogWrite(MC2, pwm2);
    digitalWrite(BR2, br2);

    printOutput();
}

void printOutput()
{
    if (millis() - lastSerialPrint >= SERIAL_UPDATE_TIME_MS)
    {
        Serial.print("SpeedSelect: ");
        Serial.print(speedSelect);
        Serial.print(", MotorSelect: ");
        Serial.print(motorSelect);
        Serial.print(" || ");
        Serial.print("Speed1: ");
        Serial.print(motor1Speed);
        Serial.print(", PWM1: ");
        Serial.print(pwm1);
        Serial.print(", Dir1: ");
        Serial.print(dir1);
        Serial.print(", Brake1: ");
        Serial.print(br1);
        Serial.print(", Speed2: ");
        Serial.print(motor2Speed);
        Serial.print(", PWM2: ");
        Serial.print(pwm2);
        Serial.print(", Dir2: ");
        Serial.print(dir2);
        Serial.print(", Brake2: ");
        Serial.println(br2);

        lastSerialPrint = millis();
    }
}

void serialCommandHandler()
{
    if (Serial.available())
    {
        String command = Serial.readStringUntil('\n');
        command.trim();
        char cmdType = command.charAt(0);

        if (cmdType == 'M')
        { // Mode switch command: "M,0" for CONTROL, "M,1" for WRITE
            int mode = command.substring(2).toInt();
            if (mode == 0)
            {
                currentMode = CONTROL;
                Serial.println("Switched to CONTROL mode");
            }
            else if (mode == 1)
            {
                currentMode = WRITE;
                Serial.println("Switched to WRITE mode");
            }
        }
        else if (cmdType == 'W')
        { // Motor update command in the format: "W,<motorSelect>,<speedSelect>"
            int firstComma = command.indexOf(',');
            int secondComma = command.indexOf(',', firstComma + 1);
            if (firstComma != -1 && secondComma != -1)
            {
                motorSelect = command.substring(firstComma + 1, secondComma).toInt();
                speedSelect = command.substring(secondComma + 1).toInt();

                // Enforce the range -100 to 100
                if (speedSelect < -100)
                    speedSelect = -100;
                if (speedSelect > 100)
                    speedSelect = 100;

                changeParam = true;
                Serial.print("Serial update: motorSelect = ");
                Serial.print(motorSelect);
                Serial.print(", speedSelect = ");
                Serial.println(speedSelect);
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

void requestEvent()
{
    Serial.print("Sending: ");
    Serial.print(motor1Speed);
    Serial.print(", ");
    Serial.println(motor2Speed);

    Wire.write(motor1Speed);
    Wire.write(motor2Speed);
}

void receiveEvent(int howMany)
{
    int i = 0;
    while (Wire.available())
    {
        commandSelect[i] = Wire.read();
        Serial.print(i);
        Serial.print("-> ");
        Serial.print(commandSelect[i]);
        Serial.print(", ");
        i++;
    }
    Serial.println();
    motorSelect = commandSelect[0];
    speedSelect = commandSelect[1];

    switch (motorSelect)
    {
    case 1:
        motor1Speed = speedSelect;
        break;
    case 2:
        motor2Speed = speedSelect;
        break;
    }

    changeParam = true;
}