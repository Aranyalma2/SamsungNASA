#ifndef NASA_COMMAND_H
#define NASA_COMMAND_H

#include <Arduino.h>
#include "NASAProtocol.h"

class NASACommand {
public:
    NASACommand();
    
    void decode(const uint8_t* data, size_t index);
    void encode(uint8_t* data, size_t index) const;
    
    bool getPacketInformation() const { return _packetInformation; }
    uint8_t getProtocolVersion() const { return _protocolVersion; }
    uint8_t getRetryCount() const { return _retryCount; }
    PacketType getPacketType() const { return _packetType; }
    DataType getDataType() const { return _dataType; }
    uint8_t getPacketNumber() const { return _packetNumber; }
    
    void setPacketInformation(bool value) { _packetInformation = value; }
    void setProtocolVersion(uint8_t value) { _protocolVersion = value; }
    void setRetryCount(uint8_t value) { _retryCount = value; }
    void setPacketType(PacketType value) { _packetType = value; }
    void setDataType(DataType value) { _dataType = value; }
    void setPacketNumber(uint8_t value) { _packetNumber = value; }
    
    static const size_t SIZE = 3;
    
private:
    bool _packetInformation;
    uint8_t _protocolVersion;
    uint8_t _retryCount;
    PacketType _packetType;
    DataType _dataType;
    uint8_t _packetNumber;
};

#endif // NASA_COMMAND_H