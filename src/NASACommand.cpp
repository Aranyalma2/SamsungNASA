#include "NASACommand.h"

NASACommand::NASACommand()
    : _packetInformation(true),
      _protocolVersion(2),
      _retryCount(0),
      _packetType(PacketType::StandBy),
      _dataType(DataType::Undefined),
      _packetNumber(0) {
}

void NASACommand::decode(const uint8_t* data, size_t index) {
    _packetInformation = ((data[index] & 0x80) >> 7) == 1;
    _protocolVersion = (data[index] & 0x60) >> 5;
    _retryCount = (data[index] & 0x18) >> 3;
    _packetType = (data[index + 1] & 0xF0) >> 4;
    _dataType = data[index + 1] & 0x0F;
    _packetNumber = data[index + 2];
}

void NASACommand::encode(uint8_t* data, size_t index) const {
    data[index] = ((_packetInformation ? 1 : 0) << 7) |
                  (_protocolVersion << 5) |
                  (_retryCount << 3);
    data[index + 1] = ((uint8_t)_packetType << 4) | (uint8_t)_dataType;
    data[index + 2] = _packetNumber;
}