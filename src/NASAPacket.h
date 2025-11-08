#ifndef NASA_PACKET_H
#define NASA_PACKET_H

#include <Arduino.h>
#include "NASAProtocol.h"
#include "NASAAddress.h"
#include "NASACommand.h"
#include "NASAMessageSet.h"

class NASAPacket {
public:
    NASAPacket();
    ~NASAPacket();
    
    bool decode(const uint8_t* data, size_t length);
    size_t encode(uint8_t* buffer, size_t maxLength);
    
    // Address access
    NASAAddress& getSourceAddress() { return _sourceAddress; }
    NASAAddress& getDestinationAddress() { return _destinationAddress; }
    const NASAAddress& getSourceAddress() const { return _sourceAddress; }
    const NASAAddress& getDestinationAddress() const { return _destinationAddress; }
    
    // Command access
    NASACommand& getCommand() { return _command; }
    const NASACommand& getCommand() const { return _command; }
    
    // Message management
    bool addMessage(const NASAMessageSet& message);
    size_t getMessageCount() const { return _messageCount; }
    const NASAMessageSet* getMessage(size_t index) const;
    void clearMessages();
    
    // Quick message access helpers
    bool hasMessage(uint16_t messageNumber) const;
    uint32_t getMessageValue(uint16_t messageNumber, uint32_t defaultValue = 0) const;
    
private:
    NASAAddress _sourceAddress;
    NASAAddress _destinationAddress;
    NASACommand _command;
    NASAMessageSet* _messages;
    size_t _messageCount;
    size_t _messageCapacity;
    
    void ensureCapacity(size_t capacity);
};

#endif // NASA_PACKET_H