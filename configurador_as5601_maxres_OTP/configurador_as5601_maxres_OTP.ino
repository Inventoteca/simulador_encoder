// Do not use this sketch unless You want program AS5601 to 2048 steps
// Very initial version
// Wersja testowa wstępna, nie do użytku. Służy wyłącznie do jednorazowego zaprogramowania AS5601 na 2049 kroków.

#include <Wire.h>
void setup()
{
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
  Wire.begin();
  Serial.begin(115200);

  //scan first
  byte error, address;
  int nDevices = 0;
  Serial.println("Scanning for I2C devices ...");
  for (address = 0x01; address < 0x7f; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf("I2C device found at address 0x%02X\n", address);
      nDevices++;
    } else if (error != 2) {
      Serial.printf("Error %d at address 0x%02X\n", error, address);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found");
  }

  //Set Quadrature (AB) resolution to 2048 - (default is 8)
  /*
    0000 : 8  - 0x00
    0001 : 16 - 0x01
    0010 : 32 - 0x02
    0011 : 64 - 0x03
    0100 : 128 - 0x04
    0101 : 256 - 0x05
    0110 : 512 - 0x06
    0111 : 1024 - 0x07
    others : 2048 - 0x08
  */
  Serial.print("settings");
  Wire.beginTransmission(0x36);       // start transmission to AS5601
  Wire.write(byte(0x09));             // sets adress to ABN
  Wire.write(byte(0x08));             // set resolution 2048 -
  Wire.endTransmission();             // stop transmitting


  Serial.print("PROGRAMOWANIE ");
  Wire.beginTransmission(0x36);       // start transmission to AS5601
  Wire.write(byte(0xFF));             // sets adress to OTP
  Wire.write(byte(0x40));             // write 0x40 to adress from previous line - killer line!!!!
  Wire.endTransmission();            // stop transmission
}

void loop() {
  int t = 2000;
  digitalWrite(18, 1);
  delay(t);
  digitalWrite(19, 1);
  delay(t);
  digitalWrite(18, 0);
  delay(t);
  digitalWrite(19, 0);
  delay(t);
}
