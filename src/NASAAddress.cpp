#include "NASAAddress.h"

NASAAddress::NASAAddress(uint8_t klass, uint8_t channel, uint8_t address)
    : _class(klass), _channel(channel), _address(address) {
}

void NASAAddress::decode(const uint8_t* data, size_t index) {
    _class = data[index];
    _channel = data[index + 1];
    _address = data[index + 2];
}

void NASAAddress::encode(uint8_t* data, size_t index) const {
    data[index] = _class;
    data[index + 1] = _channel;
    data[index + 2] = _address;
}