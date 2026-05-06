#ifndef NASA_ADDRESS_H
#define NASA_ADDRESS_H

#include <Arduino.h>

#include "NASAProtocol.h"

class NASAAddress {
   public:
    NASAAddress(uint8_t klass = AddressClass::Undefined, uint8_t channel = 0, uint8_t address = 0);

    void decode(const uint8_t* data, size_t index);
    void encode(uint8_t* data, size_t index) const;

    uint8_t getClass() const { return _class; }
    uint8_t getChannel() const { return _channel; }
    uint8_t getAddress() const { return _address; }

    void setClass(uint8_t klass) { _class = klass; }
    void setChannel(uint8_t channel) { _channel = channel; }
    void setAddress(uint8_t address) { _address = address; }

    static const size_t SIZE = 3;

   private:
    uint8_t _class;
    uint8_t _channel;
    uint8_t _address;
};

#endif  // NASA_ADDRESS_H