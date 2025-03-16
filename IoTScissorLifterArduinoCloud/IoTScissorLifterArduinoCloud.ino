#include "arduino_secrets.h"
#include "thingProperties.h"
#include <Wire.h>

#define ENA_PIN 5           // PWM pin for motor speed control
#define IN1_PIN 18          // Motor direction control pin 1
#define IN2_PIN 19          // Motor direction control pin 2
#define UP_LIMIT_PIN 22     // Up limit switch pin
#define DOWN_LIMIT_PIN 23   // Down limit switch pin
#define UP_SWITCH_PIN 27    // Up switch pin
#define DOWN_SWITCH_PIN 26  // Down switch pin
#define RAMP_INTERVAL 20    // Time interval between speed increments (in milliseconds)

int speedValue;        // Variable to store the motor speed
int direction;         // Variable to store the motor direction (0 - stop, 1 - up, 2 - down)
int currentSpeed = 0;  // Variable to store the current motor speed
int ONBOARD_LED=2; 
void setup() {
  Serial.begin(115200);
  delay(1500);

  initProperties();

  pinMode(ONBOARD_LED, OUTPUT);  // Set onboard LED pin as output

  pinMode(ENA_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(UP_LIMIT_PIN, INPUT_PULLUP);     // Set up limit switch pin as input with pull-up resistor
  pinMode(DOWN_LIMIT_PIN, INPUT_PULLUP);   // Set down limit switch pin as input with pull-up resistor
  pinMode(UP_SWITCH_PIN, INPUT_PULLUP);    // Set up switch pin as input with pull-up resistor
  pinMode(DOWN_SWITCH_PIN, INPUT_PULLUP);  // Set down switch pin as input with pull-up resistor

  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update();

  // Check if connected to cloud and control onboard LED accordingly
  if (ArduinoCloud.connected()) {
    digitalWrite(ONBOARD_LED, HIGH);  // Turn on onboard LED
    deviceStatus=1;
  } else {
    digitalWrite(ONBOARD_LED, LOW);  // Turn off onboard LED
    deviceStatus=0;
  }

  if (digitalRead(UP_LIMIT_PIN) == LOW) {
    Serial.println("upLimitLScut");
    lS1 = 1;
    stopMotor();  // Stop motor if up limit switch is triggered
  } else if (digitalRead(DOWN_LIMIT_PIN) == LOW) {
    Serial.println("downLimitLScut");
    lS2 = 1;
    stopMotor();  // Stop motor if down limit switch is triggered
  } else {
    lS1 = 0;
    lS2 = 0;
  }


  // Control motor based on direction and limit switch states
  if ((direction == 1 && digitalRead(UP_LIMIT_PIN) == LOW) || (digitalRead(UP_SWITCH_PIN) == LOW && digitalRead(UP_LIMIT_PIN) == LOW)) {
    Serial.println("upLimit");
    stopMotor();  // Stop motor if up limit switch is triggered
  } else if ((direction == 2 && digitalRead(DOWN_LIMIT_PIN) == LOW) || (digitalRead(DOWN_SWITCH_PIN) == LOW && digitalRead(DOWN_LIMIT_PIN) == LOW)) {
    Serial.println("downLimit");
    stopMotor();  // Stop motor if down limit switch is triggered
  } else {
    if (direction == 1) {
      Serial.println("Direction1");
      moveUp();
    } else if (direction == 2) {
      Serial.println("Direction2");
      moveDown();
    } else {
      if (digitalRead(UP_SWITCH_PIN) == LOW && direction == 0) {
        Serial.println("Direction1");
        moveUp();
      } else if (digitalRead(DOWN_SWITCH_PIN) == LOW && direction == 0) {
        Serial.println("Direction2");
        moveDown();
      } else if (digitalRead(UP_SWITCH_PIN) != LOW || digitalRead(DOWN_SWITCH_PIN) != LOW && direction == 0) {
        Serial.println("Stop");
        stopMotor();
      }
      //stopMotor();
    }
  }
}

void onSpeedChange() {
  speedValue = speed;  // Update speed value
}

void onUpDownChange() {
  direction = upDown;
  Serial.println(direction);
  currentSpeed = 50;  // Reset current speed when direction changes
}

void moveUp() {
  digitalWrite(IN1_PIN, HIGH);  // Set direction to move up
  digitalWrite(IN2_PIN, LOW);
  increaseSpeed();  // Increase speed gradually
}

void moveDown() {
  digitalWrite(IN1_PIN, LOW);  // Set direction to move down
  digitalWrite(IN2_PIN, HIGH);
  increaseSpeed();  // Increase speed gradually
}

void stopMotor() {
  digitalWrite(IN1_PIN, LOW);  // Stop the motor
  digitalWrite(IN2_PIN, LOW);
}

void increaseSpeed() {
  if (currentSpeed < speedValue) {
    currentSpeed++;  // Increment speed gradually
    analogWrite(ENA_PIN, currentSpeed);
    delay(RAMP_INTERVAL);
  }
}
