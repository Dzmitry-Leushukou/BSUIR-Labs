#include <Wire.h>

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(A4, INPUT_PULLUP);  // SDA pull-up
  pinMode(A5, INPUT_PULLUP);  // SCL pull-up
  Serial.begin(9600);
  while (!Serial) { }
  Serial.println("I2C scanner...");
  Wire.begin();               // A4/A5 on Nano
}

void loop() {
  bool found = false;
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found I2C device at 0x");
      if (address < 16) Serial.print('0');
      Serial.println(address, HEX);
      found = true;
    }
  }
  if (!found) Serial.println("No I2C devices found.");
  digitalWrite(LED_BUILTIN, found ? HIGH : LOW);
  delay(2000);
}
