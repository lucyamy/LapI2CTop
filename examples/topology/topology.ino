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

void setup() {
  Serial.begin(9600);
  delay(500);
  i2c.begin();
}

void loop() {
  unsigned long startTime, runTime;
  // Build the topology of the I2C bus. LapI2CTop doesn't
  // expose this directly, but we can use calls to the
  // library to find what we need.
  Serial.println();
  Serial.println("Discover device topology");
  startTime = millis();
  numberOfDevices = i2c.rescan();
  runTime = millis() - startTime;
  Serial.print("Found ");
  Serial.print(numberOfDevices);
  Serial.print(" devices in ");
  Serial.print(runTime);
  Serial.print(" milliseconds, having scanned for ");
  Serial.print(i2c.getScans());
  Serial.println(" potential devices");
  numberOfDevices = i2c.rescan();
  dumpTopology();
  delay(20000);
}

void dumpDevice(uint8_t dev) {
  if(i2c.getParent(dev)) {
    dumpDevice(i2c.getParent(dev));
    Serial.print("=>(Mux:0x");
    Serial.print(i2c.getAddress(i2c.getParent(dev)), HEX);
    Serial.print(",Bus:");
    Serial.print(i2c.getBus(dev));
    Serial.print(')');
  }
}

void dumpTopology() {
  Serial.println("I2C bus topology:");
  for(uint8_t i = 1; i <= numberOfDevices; i++) {
    Serial.print("[Device ");
    if(i < 10) Serial.print('0');
    Serial.print(i);
    Serial.print("]");
    dumpDevice(i);
    Serial.print("->Address:");
    Serial.println(i2c.getAddress(i), HEX);
  }
}
