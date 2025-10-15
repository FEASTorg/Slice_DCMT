#include <Wire.h>
//#include <SPI.h>

#define I2C_ADR 14 //Set this however you want

#define DIR1  5
#define MC1   6
#define BR1   7
#define DIR2  9
#define MC2   10
#define BR2   11

bool dir1 = 1;
bool dir2 = 1;
bool br1 = 0;
bool br2 = 0;

int pwm1, pwm2;

typedef union //Define a float that can be broken up and sent via I2C
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

int blinkPeriod = 1000;
long lastBlinkTime = 0;
bool blinkOn = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(MC1, OUTPUT);
  pinMode(MC2, OUTPUT);
  pinMode(DIR1, OUTPUT);
  pinMode(DIR2, OUTPUT);
  pinMode(BR1, OUTPUT);
  pinMode(BR2, OUTPUT);

  Serial.begin(9600);
  Wire.begin(I2C_ADR);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}

void loop() {
  if(millis() - lastBlinkTime >= blinkPeriod){
    lastBlinkTime = millis();
    if(blinkOn){
      digitalWrite(LED_BUILTIN, LOW);
      blinkOn = false;
    }
    else{
      digitalWrite(LED_BUILTIN, HIGH);
      blinkOn = true;
    }
  }
  if(changeParam){
    switch(motorSelect){
      case 1: // Motor 1 edit
        if(speedSelect == 0){
          br1 = 1;
          pwm1 = 0;
          Serial.println("Motor 1 brake");
        }
        else if(speedSelect < 0 && speedSelect >= -100){
          br1 = 0;
          dir1 = 0;
          speedSelect *= -1;
          pwm1 = map(speedSelect, 0, 100, 0, 255);
          Serial.print("Motor 1 back ");
          Serial.println(pwm1);
        }
        else if(speedSelect > 0 && speedSelect <= 100){
          br1 = 0;
          dir1 = 1;
          pwm1 = map(speedSelect, 0, 100, 0, 255);
          Serial.print("Motor 1 forward ");
          Serial.println(pwm1);
        }
        else{
          Serial.println("Invalid Entry, nothing will be changed");
        }
        break;
  
      case 2: // motor 2 edit
        if(speedSelect == 0){
          br2 = 1;
          pwm2 = 0;
          Serial.println("Motor 2 brake");
        }
        else if(speedSelect < 0 && speedSelect >= -100){
          br2 = 0;
          dir2 = 0;
          speedSelect *= -1;
          pwm2 = map(speedSelect, 0, 100, 0, 255);
          Serial.print("Motor 2 back ");
          Serial.println(pwm2);
        }
        else if(speedSelect > 0 && speedSelect <= 100){
          br2 = 0;
          dir2 = 1;
          pwm2 = map(speedSelect, 0, 100, 0, 255);
          Serial.print("Motor 2 forward ");
          Serial.println(pwm2);
        }
        else{
          Serial.println("Invalid Entry, nothing will be changed");
        }
        break;
        
      default:
        Serial.println("oopsie poopsie, that's not a command, silly");
        break;
    }
    digitalWrite(DIR1, dir1);
    analogWrite(MC1, pwm1);
    
    digitalWrite(DIR2, dir2);
    analogWrite(MC2, pwm2);

    changeParam = false;
  }
}

void requestEvent() {
  Serial.print("Sending: ");
  Serial.print(motor1Speed);
  Serial.print(", ");
  Serial.println(motor2Speed);

  Wire.write(motor1Speed);
  Wire.write(motor2Speed);
}

void receiveEvent(int howMany) {
  int i=0;
  while (Wire.available()) {
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

  switch(motorSelect){
    case 1:
      motor1Speed = speedSelect;
      break;
    case 2:
      motor2Speed = speedSelect;
      break;
  }
  
  changeParam = true;
}
