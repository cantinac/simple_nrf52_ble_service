/*
  nrF52 Feather Simple Custom Service Example

  This is fairly simple example for creating and using a custom BLE service and
  characteristics on the Adafruit Feather nRF52. It's meant to be instructive,
  so it's value is more educational that practical.

  For additional information, see:

  Learning Guide for the Adafruit Feather nRF52
  https://learn.adafruit.com/bluefruit-nrf52-feather-learning-guide

  Github Repo for the Adafruit Feather nRF2 board support and API
  https://github.com/adafruit/Adafruit_nRF52_Arduino

  Note that the Learning Guide API docs trail the actual code by a bit.
  If you want the latest API, check the Github repo.
*/

#include <Arduino.h>
#include <bluefruit.h>
#include "service_uuids.h"

// Create a custom BLE service, using the UUID defined in the header
BLEService customBleService = BLEService(UUID_SVC_EXAMPLE);

// Create the characteristics of the service
BLECharacteristic counterChr = BLECharacteristic(UUID_CHR_COUNTER);
BLECharacteristic toggleChr = BLECharacteristic(UUID_CHR_TOGGLE);
BLECharacteristic dataChr = BLECharacteristic(UUID_CHR_DATA);

// "Manufacturer" data - explained later, but it's really just random
uint8_t manufacturerData[4] = {0x00, 0x12, 0x4D, 0xC3};

// Some variables to hold state
uint32_t counter = 0; // Initial state
bool toggle = 0;
uint8_t data[4] = {0x00, 0x00, 0x00, 0x00};

/*
  Start up the service, define the details of the characteristics, and bind them
  to the service.

  In the bluefruit library, you have to follow very specific ordering:

  1. Start the service
  2. Define a characteristic
  3. Start the characteristic
  4. Report 2, 3 until you've got the service fully defined
  5. [optional] loop back to 1 until you're done defining service_uuids

  To make this clear and clean, wrap the service and related characteristic definition
  in a function, that will get invoked somewhere else (often in setup()).
*/
void setupService() {
  // Start the service
  customBleService.begin();
  /*
    We use setProperties to determine the access for the characteristic. The flags
    are ORed together to set permissions for reading, writing, etc.

    CHR_PROPS_BROADCAST       broadcast the property in the ad packet
    CHR_PROPS_READ            make the property readable
    CHR_PROPS_WRITE           make the property writeable
    CHR_PROPS_WRITE_WO_RESP   make the property writeable with no response (useless for things like control events)
    CHR_PROPS_NOTIFY          make the property capable of issuing notificaton of change to the central
    CHR_PROPS_INDICATE        like notify, but the sender also acknowledge the new data back to the peripheral

  */
  // The counter will be read-only
  counterChr.setProperties(CHR_PROPS_READ);
  /*
    Now we determine the security access for the characteristic. This effects both access to the
    characteristic and the binding behavior of the central. For example, if a characteristic is
    encrypted or signed, an iPhone acting as a central will need to pair with the peripheral before
    it can perform the corresponding action.
    
    Note that it's possible to have different security on the read and write aspects of the characteristic,
    such as an open read with an encrypted write.

    SECMODE_NO_ACCESS         block access to an operation
    SECMODE_OPEN              open access, use for BLE "Just Works" operation
    SECMODE_ENC_NO_MITM       encrypted, does not prevent man-in-the-middle sniffing
    SECMODE_ENC_WITH_MITM     encrypted, with secure key exchange to prevent MITM
    SECMODE_SIGNED_NO_MITM    signed, with no MITM protection
    SECMODE_SIGNED_WITH_MITM  signed, with MITM

    We typically choose SECMODE_NO_ACCESS to block access to the characteristic, or SECMODE_OPEN to allow acces
    via BLE "Just Works", which allows communications between centrals and peripherals without explicit pairing
    and bonding.

    Note: encrypting or signing BLE characteristics is only really useful when used with MITM protection, and
    even then, the encrpytion used by BLE has been cracked! It does provide some modicum of protection from
    leaking data, but don't consider it ironclad.
  */
  // Make the counter openly readable, while blocking write operations entirely
  counterChr.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);

  // Set the "size" of the characteristic variable, in bytes. In this case, 4 bytes/32 bits
  counterChr.setFixedLen(4);

  // Set a human-readable descriptor; this help when inspecting the service
  counterChr.setUserDescriptor("counter");

  // Now 'bind' the characteristic to the service by starting it
  counterChr.begin();
  
  // Make the toogle readable, writeable, and allow subscribing for notification
  toggleChr.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE | CHR_PROPS_NOTIFY );
  // Make toogle open access for reads and writes
  toggleChr.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  toggleChr.setUserDescriptor("toggle");
  toggleChr.begin();

  // Make the data readable and writable
  dataChr.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  // Secure the data with encryption and MITM protection
  dataChr.setPermission(SECMODE_ENC_WITH_MITM, SECMODE_ENC_WITH_MITM);
  dataChr.setUserDescriptor("data");
  dataChr.begin();
}

void startAdvertising() {
  // Make the peripheral generally discoverable
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  // Include the current transmission power
  Bluefruit.Advertising.addTxPower();
  /*
    Advertise our customer service UUID; this is important, as peripherals can use it
    to ID our class of devices. In fact, it's required for iOS background scanning
    to work with our advertising.
  */
  Bluefruit.Advertising.addService(customBleService);
  // Add the name of our decice the scan response (the secondary scan data packet provided on request)
  Bluefruit.ScanResponse.addName();
  // Add our manufacturer data to the scan response
  /*
    Manufacturer data is information that can be included in the ad packet or scan response for BLE.
    It's pretty much up to the creator of a device to decide what goes into the manufacture data,
    and what it means. Here, we'll just add some to the scan response to demonstrate how it's done.
  */
  Bluefruit.ScanResponse.addManufacturerData(manufacturerData, sizeof(manufacturerData));
  // Restart advertising after disconnection from a central
  Bluefruit.Advertising.restartOnDisconnect(true);
  // Set up callbacks for connect and disconnection from centrals
  Bluefruit.setConnectCallback(connectCallback);
  Bluefruit.setDisconnectCallback(disconnectCallback);
  // Set up some advertising parameters
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
}

void connectCallback(uint16_t handle) {
  // handle checks for running sessions and other reconnection details here
  char central[32] = { 0 };
  Bluefruit.Gap.getPeerName(handle, central, sizeof(central));
  Serial.print("Connected to ");
  Serial.println(central);
  // update our counter on each connect
  counter = counter + 1;
  // Set the characteristic
  counterChr.write32(counter);
}

void disconnectCallback(uint16_t handle, uint8_t reason) {
  // handle anything disconnection details here
  Serial.println("Disconnected, resuming advertising");
}

// Arduino lifecycle
void setup() {
  Serial.begin(115200);

  // Start up BLE on the Feather
  Bluefruit.begin();

  // Set up general BLE information
  Bluefruit.setName("BLE Periperal");

  // Start advertising
  startAdvertising();

  // Invoke our service setup
  setupService();
}

void loop() {

}
