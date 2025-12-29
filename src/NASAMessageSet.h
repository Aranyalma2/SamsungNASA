#ifndef NASA_MESSAGE_SET_H
#define NASA_MESSAGE_SET_H

#include <Arduino.h>
#include "NASAProtocol.h"

class NASAMessageSet {
public:
    NASAMessageSet(uint16_t messageNumber = 0);
    
    size_t decode(const uint8_t* data, size_t index);
    size_t encode(uint8_t* data, size_t index) const;
    
    uint16_t getMessageNumber() const { return _messageNumber; }
    uint8_t getType() const { return _type; }
    uint32_t getValue() const { return _value; }
    
    void setMessageNumber(uint16_t value);
    void setValue(uint32_t value) { _value = value; }
    
    size_t getSize() const { return _size; }
    
private:
    uint16_t _messageNumber;
    uint8_t _type;
    uint32_t _value;
    size_t _size;
    
    void updateType();
};

#endif // NASA_MESSAGE_SET_H