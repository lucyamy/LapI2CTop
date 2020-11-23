#include <LapI2CTop.h>

// Set this to -1 if you haven't connected the TCA9548
// reset pins to the Arduino - or just invoke LapI2CTop
// as:
//   LapI2CTop i2c;
//
#define RESET_PIN 45

LapI2CTop i2c(RESET_PIN);

// The number of I2C devices found
uint8_t numberOfDevices;

// A list of I2C device addresses we're expecting to
// find, terminated with zero. Change this by putting
// in the addresses on your bus. It's not needed by the
// library, and as shown below we can easily get the
// addresses LapI2CTop actually found.
byte addresses[] = { 0x3c, 0x40, 0x70, 0x72, 0 };

void setup() {
  byte i;
  byte device;
  Serial.begin(9600);
  delay(500);
  Serial.println("Discover device topology");
  numberOfDevices = i2c.begin();
  Serial.print("Found ");
  Serial.print(numberOfDevices);
  Serial.println(" devices.");
  Serial.println("Using getaddress() to see what we have");
  for(i = 1; i <= numberOfDevices; i++) {
    Serial.print("DeviceID ");
    Serial.print(i);
    Serial.print(" has address 0x");
    Serial.println(i2c.getAddress(i), HEX);
  }
  i = 0;
  Serial.println("And then using getID() to get the same thing");
  Serial.println("in a different form");
  while(addresses[i]) {
    device = 0;
    while((device = i2c.getID(addresses[i], device))) {
      Serial.print("DeviceID ");
      Serial.print(device);
      Serial.print(" has address 0x");
      Serial.println(addresses[i], HEX);
    }
    i++;
  }
}

void loop() {
  // Here you could actually do something with I2C. I guess :).
}
