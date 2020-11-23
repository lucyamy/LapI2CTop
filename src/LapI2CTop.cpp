#include <Arduino.h>
#include <Wire.h>
#include "LapI2CTop.h"

#define getDeviceAddress(x) (devices[(x) - 1].address)
#define getDeviceParent(x) (devices[(x) - 1].parentID)
#define getDeviceBus(x) (devices[(x) - 1].bus)

//
// Constructors
//
//


LapI2CTop::LapI2CTop(void) {
  wire = &Wire;
  muxPin = -1;
  lastDeviceID = 0;
}

LapI2CTop::LapI2CTop(uint8_t muxReset) {
  wire = &Wire;
  muxPin = muxReset;
  lastDeviceID = 0;
}

LapI2CTop::LapI2CTop(TwoWire *wi) {
  wire = wi;
  muxPin = -1;
  lastDeviceID = 0;
}

LapI2CTop::LapI2CTop(uint8_t muxReset, TwoWire *wi) {
  wire = wi;
  muxPin = muxReset;
  lastDeviceID = 0;
}

//
// Public functions
//

//
// begin(). Does the topology discovery
//

uint16_t LapI2CTop::begin() {
  wire->begin();
  return rescan();
}

//
// Bus control functions
//

uint8_t LapI2CTop::setBus(uint16_t deviceID, bool force) {
  if(force) lastDeviceID = 0;
  return setBus(deviceID);
}

uint8_t LapI2CTop::setBus(uint16_t deviceID) {
  if(deviceID > deviceTotal) return 1;
  if(lastDeviceID == deviceID) return 0;
  if(lastDeviceID) disableBus(lastDeviceID);
  lastDeviceID = deviceID;
  if(getDeviceParent(deviceID)) {
    setBus(getDeviceParent(deviceID));
    _setBus(getDeviceAddress(getDeviceParent(deviceID)), getDeviceBus(deviceID));
  }
  return 0;
}

uint8_t LapI2CTop::disableBus(uint16_t deviceID, bool force) {
  if(force) setBus(deviceID);
  return disableBus(deviceID);
}

uint8_t LapI2CTop::disableBus(uint16_t deviceID) {
  if(deviceID > deviceTotal) return 1;
  if(lastDeviceID != deviceID) return 2;
  if(getDeviceParent(deviceID)) {
    _disableBus(getDeviceAddress(getDeviceParent(deviceID)));
    disableBus(getDeviceParent(deviceID));
  }
  lastDeviceID = 0;
  return 0;
}

//
// Discovery functions
//

uint16_t LapI2CTop::rescan() {
  if(muxPin >= 0) {
    pinMode(muxPin, OUTPUT);
    digitalWrite(muxPin, LOW);
    delayMicroseconds(50);
    digitalWrite(muxPin, HIGH);
  }
  busDevice = 0;
  totalScans = 0;
  deviceTotal = discoverTopology(0, 0, 0, 0);
  return deviceTotal;
}

uint8_t LapI2CTop::getAddress(uint16_t devID) {
  return getDeviceAddress(devID);
}

uint8_t LapI2CTop::getBus(uint16_t devID) {
  return getDeviceBus(devID);
}

uint16_t LapI2CTop::getParent(uint16_t devID) {
  return getDeviceParent(devID);
}

uint16_t LapI2CTop::getID(uint8_t address, uint16_t devID) {
  for(uint16_t i = ++devID; i <= deviceTotal; i++) {
    if(getDeviceAddress(i) == address) return i;
  }
  return 0;
}

uint32_t LapI2CTop::getScans(void) {
  return totalScans;
}

//
// Private functions
//

//
// Bus control functions
//

inline void LapI2CTop::_setBus(uint8_t mux, uint8_t bus) {
  wire->beginTransmission(mux);
  wire->write(1 << bus);
  wire->endTransmission();
}

inline void LapI2CTop::_disableBus(uint8_t mux) {
  wire->beginTransmission(mux);
  wire->write(0);
  wire->endTransmission();
}

//
// Discovery functions
//

uint16_t LapI2CTop::addDevice(uint8_t address, uint16_t parent, uint8_t bus) {
  devices[busDevice].address = address;
  devices[busDevice].parentID = parent;
  devices[busDevice].bus = bus;
  busDevice++;
  return busDevice;
}

uint16_t LapI2CTop::discoverTopology(uint8_t level, uint16_t parent, uint8_t bus, uint8_t foundSoFar) {
  uint8_t dev;
  uint16_t nld, lastDevice = 0;
  uint8_t thisLevelMuxes[8], thisLevelCount = 0;
  bool skip = false;
  dev = 0x0c;
  for(;;) {
    skip = false;
    if(skip == false) {
      dev = i2cScanAddresses(dev);
      if(!dev) {
        break;
      } else {
        for(uint8_t l = 0; l < foundSoFar; l++) {
          if(foundAddresses[l] == dev) {
            skip = true;
            break;
          }
        }
        if(!skip) {
            if(dev >= 0x70 && dev <= 0x77) {
              lastDevice = thisLevelMuxes[thisLevelCount++] = addDevice(dev, parent, bus);
            } else {
              lastDevice = addDevice(dev, parent, bus);
            }
          foundAddresses[foundSoFar++] = dev;
        }
      }
    }
    dev++;
  }
  for(uint8_t i = 0; i < thisLevelCount; i++) {
    for(uint8_t bus = 0; bus < 8; bus++) {
      _setBus(getDeviceAddress(thisLevelMuxes[i]), bus);
      nld = discoverTopology(level + 1, thisLevelMuxes[i], bus, foundSoFar);
      if(nld) lastDevice = nld;
    }
    _disableBus(getDeviceAddress(thisLevelMuxes[i]));
  }
  return lastDevice;
}

inline uint8_t LapI2CTop::scanDevice(uint8_t device) {
  totalScans++;
  wire->beginTransmission(device);
  return wire->endTransmission();
}

inline uint8_t LapI2CTop::i2cScanAddresses(uint8_t fromAddress) {
  byte address;
  if(fromAddress < 0x07) fromAddress = 0x07;
  for(address = fromAddress; address <= 0x77; address++) {
    if(!scanDevice(address)) {
      return address;
    }
  }
  return 0;
}
