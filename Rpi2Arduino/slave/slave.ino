#include <Wire.h>

void setup() {
  Wire.begin(8); // Slave address
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);
}

void loop() {
  delay(100);
}

void receiveEvent(int howMany) {
  String msg = "";
  while (Wire.available()) {
    char c = Wire.read();
    msg += c;
  }
  Serial.println(msg); // Display motion sensor data
}
