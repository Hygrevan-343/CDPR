#include <Wire.h>

void setup() {
  Wire.begin(); // I2C Master
  Serial.begin(9600); // From ESP
}

void loop() {
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();
    Wire.beginTransmission(8); // Address of actual Arduino Uno
    Wire.write(msg.c_str());
    Wire.endTransmission();
  }
}
