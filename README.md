# LapI2CTop
LapI2CTop supports a branched topology I2C network, using the ubiquitous TCA9748 multiplexers. It performs discovery to find each I2C device on the bus, and assigns each a deviceID that can be used to setup the path to that device. There is a configurable limit of 64 I2C devices. If you need more you can change that. All that is needed to access a device anywhere on the bus is a call to **setbus(deviceID)** before talking to it. You then use the usual **Wire** calls to access the device. It's that simple.

If you connect the multiplexers' reset pins to the Arduino, **LapI2CTop** will reset them before performing discovery. This removes the potential problem of I2C bus paths through multiplexers being enabled that the library doesn’t know about, which could cause issues. If you’re performing discovery right after powering on the system, that won’t be a problem, of course, as the multiplexers will reset themselves.

**LapI2CTop** is used with I2C busses set up like this (for example. The TCA9548s have 8 multiplexed busses, but there are only 7 in the drawing to keep it neat):

     .-------.
     |   A   |          .-----.        .-----.
     |   r   |----+-----|  9  |--------|  9  |--D
     |   d   |    |  D--|  5  |--D  D--|  5  |--D
     |   u   |    |  D--|  4  |--D  D--|  4  |--D 
     |   i   |    |  D--|  8  |--D  D--|  8  |--D
     |   n   |    |     `-----'        `-----'
     |   o   |    |     .-----.        .-----.
     `-------'    `-----|  9  |--------|  9  |--D
                     D--|  5  |--D  D--|  5  |--D
                     D--|  4  |--D  D--|  4  |--D 
                     D--|  8  |--`  D--|  8  |--D
                        `-----'  |     `-----'
                                 |
                                 |     .-----.
                                 `-----|  9  |--D
                                    D--|  5  |--D
                                    D--|  4  |--D 
                                    D--|  8  |--D
                                       `-----'
  
    Where 'D' represents an I2C device.

**Constructor**
---------------------
**LapI2CTop** is invoked as:

  **LapI2CTop (uint8_t *muxResetPin*, TwoWire \**wire*);** 

Neither parameter is required. If you don’t specify a reset pin, or give one of -1, then **LapI2CTop** won’t reset the multiplexers. If you don’t specify a Wire interface, it will use **Wire**. So to use **Wire1**, invoke **LapI2CTop** as:

  **LapI2CTop (&Wire1);** 

Remember to include any needed library for this. **LapI2CTop** automatically includes the **Wire** library.


**Library functions**
---------------------

The functions provided by **LapI2CTop** are:

uint16_t begin();
----------------- 

This must be called before using any other function. It initialises the **Wire** library, and performs the discovery. It returns the number of devices found, and the device numbers (**deviceID**s in **LapI2CTop** talk) allocated to them are from one to this number.

uint16_t rescan();
----------------------
This repeats the scan performed by **begin()**. If you have a hot-pluggable bus, you can call this, but be aware that deviceIDs may well change if you do.

uint8_t getAddress(uint16_t *deviceID*);
-----------------------------------
Returns the I2C address of the given deviceID.  

uint8_t getBus(uint16_t *deviceID*);
-------------------------------

Returns the TCA9548 bus number that is used to access this device.

uint16_t getParent(uint16_t *deviceID*);
-----------------------------------
Returns the deviceID of the parent device, that is, the multiplexer that this device is connected to. 

uint16_t getID(uint8_t *address*, uint16_t *deviceID*);
------------------------------------------------
This is used to find the deviceID(s) of devices you have on the bus. It returns the deviceID of a device that has the I2C address you specified. It will return the first – lowest numbered – deviceID that matches, as long as that deviceID is greater than the deviceID you pass as the second parameter. Passing zero as the parameter – which is not a valid deviceID – tells it to return the first one.

So if you have four OLED displays connected, all with the I2C address 0x3c, you can get their deviceIDs using the following code:

     oled1 = i2c.getID(0x3c, 0);
     oled2 = i2c.getID(0x3c, oled1);
     oled3 = i2c.getID(0x3c, oled2);
     oled4 = i2c.getID(0x3c, oled3);

uint8_t setBus(uint16_t *deviceID*);
----------------------------------
uint8_t setBus(uint16_t *deviceID*, bool *force*);
----------------------------------------------
This is used to set up the bus to point to a device before you talk to it with I2C. After making this call, the device specified will be reachable, and you can use the normal **Wire** functions to access it. That is the essence of this library. That’s really all you need to do to use the bus.
It will return 1 if the deviceID is invalid, otherwise 0.

**Notes:**

If the deviceID you give it is the same as the last one you gave it, it will just return. There’s nothing that needs to be done. However, if you want it to redo the task, you can pass the optional **force** parameter, which if true will force **setBus()** to redo the setup.

uint8_t disableBus(uint16_t *deviceID*);
--------------------------------------
uint8_t disableBus(uint16_t *deviceID*, bool *force*);
----------------------------------------------
There’s never usually a need to call this. It explicitly tears down the path to the device that **setBus()** set up. But if you want to make sure that all the multiplexers have all their busses disabled when not actually in use, it’s here for you. It will return 1 if the deviceID is invalid, otherwise 0.

**Notes:**

If the deviceID you give it is not the same as the last one you gave **setBus** (i.e. the path is not set up), it will return 2, to indicate that. It won’t actually do anything. However, if you want it to make sure that all elements of the path are disabled, you can pass the optional **force** parameter, which if true will force **disableBus()** to do that. It does it by first explicitly making sure the path is set up, then removing it. 

There’s one final call in the library. It’s not one that would normally be called, but it’s there for anyone interested, and that’s this one:

uint32_t getScans();
------------------------
This returns the number of I2C devices that the discovery process scanned for. On even a fairly simple setup it can be quite a large number. So if that’s something you’d like to know, make the call.

Using **LapI2CTop**
-------------------
Here is one of the example sketches that comes with the library. It demonstrates the use of the most common calls:


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

And here’s the output of that sketch, when run on a simple test setup:
    
    Discover device topology
    Found 8 devices in 243 milliseconds, having scanned for 1836 potential devices
    I2C bus topology:
    [Device 01]->Address:70
    [Device 02]=>(Mux:0x70,Bus:1)->Address:3C
    [Device 03]=>(Mux:0x70,Bus:4)->Address:72
    [Device 04]=>(Mux:0x70,Bus:4)=>(Mux:0x72,Bus:2)->Address:40
    [Device 05]=>(Mux:0x70,Bus:4)=>(Mux:0x72,Bus:4)->Address:3C
    [Device 06]=>(Mux:0x70,Bus:4)=>(Mux:0x72,Bus:7)->Address:3C
    [Device 07]=>(Mux:0x70,Bus:6)->Address:3C
    [Device 08]=>(Mux:0x70,Bus:7)->Address:3C

    
