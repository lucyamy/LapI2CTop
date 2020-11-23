#ifndef LAPI2CTOP_H
#define LAPI2CTOP_H
#include <Arduino.h>
#include <Wire.h>

// The maximum number of I2C devices on the bus.
// Up to 64, LapI2CTop needs 2 bytes per device. From 64
// to 256, that becomes 3 bytes. And from 256 to 65536
// devices, it's 4 bytes. It stops working after that :).
#define LAPI2CTOP_MAXDEVICES 64

// That maximum number of devices that can be on the bus at
// the same time. That's all the devices on the top level,
// and each mux on the path to any device, and all the devices
// at the end of the path. 16 is plenty. It can go up to 256
// without breaking, but you CANNOT have that many devices on
// at once anyway. The cost is one byte per potential device.
#define LAPI2CTOP_MAXBRANCH 16

class LapI2CTop {

  public:
    LapI2CTop(void);
    LapI2CTop(uint8_t muxReset);
    LapI2CTop(TwoWire *wire);
    LapI2CTop(uint8_t muxReset, TwoWire *wire);
    uint16_t begin(void);
    uint16_t rescan(void);
    uint8_t getAddress(uint16_t devID);
    uint8_t getBus(uint16_t devID);
    uint16_t getParent(uint16_t devID);
    uint16_t getID(uint8_t address, uint16_t devID);
    uint32_t getScans(void);
    uint8_t setBus(uint16_t deviceID);
    uint8_t setBus(uint16_t deviceID, bool force);
    uint8_t disableBus(uint16_t deviceID);
    uint8_t disableBus(uint16_t deviceID, bool force);

  private:
    uint8_t scanDevice(uint8_t device);
    void _setBus(uint8_t mux, uint8_t bus);
    void _disableBus(uint8_t mux);
    uint8_t i2cScanAddresses(uint8_t fromAddress);
    uint16_t addDevice(uint8_t address, uint16_t parent, uint8_t bus);
    uint16_t discoverTopology(uint8_t level, uint16_t parent, uint8_t bus, uint8_t foundSoFar);

    uint16_t lastDeviceID;
    uint16_t busDevice;
    uint16_t deviceTotal;
    uint32_t totalScans;
    int8_t muxPin;
    TwoWire *wire;
    uint8_t foundAddresses[LAPI2CTOP_MAXBRANCH];
#if(LAPI2CTOP_MAXDEVICES <= 64)
    struct DEVICEMAP {
      uint8_t address:7;
      uint8_t parentID:6;
      uint8_t bus:3;
    };
#elif(LAPI2CTOP_MAXDEVICES <= 256)
    struct DEVICEMAP {
      uint8_t address;
      uint8_t parentID;
      uint8_t bus;
    };
#else
    struct DEVICEMAP {
      uint8_t address;
      uint16_t parentID;
      uint8_t bus;
    };
#endif
    DEVICEMAP devices[LAPI2CTOP_MAXDEVICES];
};

#endif
