#include <Arduino.h>
/* 
  The above include exists so we get types when working in an outside editor like VSCode. It's not
  needed when working in the Arduino IDE, but it doesn't hurt either.
*/

/*
  custom service UUIDS

  For each custom service, you need at least the service UUID. You should also have a unique UUID for
  each characteristic of that service.

  Defining these values in separate header helps keep the code cleaner.

  The UUIDs are 128 bits, expressed as an array of sixteen 1 byte (8bit) values in hex. 

  The service and characteristic UUIDs are not really related in any way and could be any 128 bit values.
  By convention, we will make the characteristic UUIDs an informal enumeration, with the first 12 bytes
  derived from the service UUID and the last four bytes zeroed, then incrementing the values starting at one.
  This provides a pretty large potential address space, certainly large enough for the few characteristics
  we are likely to define on a real service.

  Also, note the array of values is presented in reverse order. In our example, the reported service UUID will
  be cf636ca6-ab07-4506-bc67-93192fb9da18.
*/

// Custom serive UUID
const uint8_t UUID_SVC_EXAMPLE[16] = {0x18,0xDA,0xB9,0x2F,0x19,0x93,0x67,0xBC,0x6,0x45,0x7,0xAB,0xA6,0x6C,0x63,0xCF};

// Characteristic IDs for the service
const uint8_t UUID_CHR_COUNTER[16] = {0x1,0x0,0x0,0x0,0x19,0x93,0x67,0xBC,0x6,0x45,0x7,0xAB,0xA6,0x6C,0x63,0xCF};
const uint8_t UUID_CHR_TOGGLE[16] = {0x2,0x0,0x0,0x0,0x19,0x93,0x67,0xBC,0x6,0x45,0x7,0xAB,0xA6,0x6C,0x63,0xCF};
const uint8_t UUID_CHR_DATA[16] = {0x3,0x0,0x0,0x0,0x19,0x93,0x67,0xBC,0x6,0x45,0x7,0xAB,0xA6,0x6C,0x63,0xCF};
