#include "NASAPacket.h"

// CRC16 implementation moved from NASAProtocol
uint16_t NASAPacket::crc16(const uint8_t* data, size_t startIndex, size_t length) {
    uint16_t crc = 0;
    for (size_t index = startIndex; index < startIndex + length; index++) {
        crc = crc ^ (data[index] << 8);
        for (int i = 0; i < 8; i++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc & 0xFFFF;
}

NASAPacket::NASAPacket()
    : _messages(nullptr), _messageCount(0), _messageCapacity(0) {
}

NASAPacket::~NASAPacket() {
    if (_messages) {
        delete[] _messages;
    }
}

NASAPacket::NASAPacket(const NASAPacket& other)
    : _sourceAddress(other._sourceAddress),
      _destinationAddress(other._destinationAddress),
      _command(other._command),
      _messages(nullptr),
      _messageCount(other._messageCount),
      _messageCapacity(other._messageCapacity) {
    if (other._messages && _messageCapacity > 0) {
        _messages = new NASAMessageSet[_messageCapacity];
        for (size_t i = 0; i < _messageCount; i++) {
            _messages[i] = other._messages[i];
        }
    }
}

NASAPacket& NASAPacket::operator=(const NASAPacket& other) {
    if (this != &other) {
        if (_messages) {
            delete[] _messages;
            _messages = nullptr;
        }
        _sourceAddress = other._sourceAddress;
        _destinationAddress = other._destinationAddress;
        _command = other._command;
        _messageCount = other._messageCount;
        _messageCapacity = other._messageCapacity;
        if (other._messages && _messageCapacity > 0) {
            _messages = new NASAMessageSet[_messageCapacity];
            for (size_t i = 0; i < _messageCount; i++) {
                _messages[i] = other._messages[i];
            }
        }
    }
    return *this;
}

NASAPacket::NASAPacket(NASAPacket&& other) noexcept
    : _sourceAddress(std::move(other._sourceAddress)),
      _destinationAddress(std::move(other._destinationAddress)),
      _command(std::move(other._command)),
      _messages(other._messages),
      _messageCount(other._messageCount),
      _messageCapacity(other._messageCapacity) {
    other._messages = nullptr;
    other._messageCount = 0;
    other._messageCapacity = 0;
}

NASAPacket& NASAPacket::operator=(NASAPacket&& other) noexcept {
    if (this != &other) {
        if (_messages) {
            delete[] _messages;
        }
        _sourceAddress = std::move(other._sourceAddress);
        _destinationAddress = std::move(other._destinationAddress);
        _command = std::move(other._command);
        _messages = other._messages;
        _messageCount = other._messageCount;
        _messageCapacity = other._messageCapacity;

        other._messages = nullptr;
        other._messageCount = 0;
        other._messageCapacity = 0;
    }
    return *this;
}

bool NASAPacket::decode(const uint8_t* data, size_t length) {
    if (data[0] != NASA_START_BYTE) {
        return false;
    }

    if (length < NASA_MIN_PACKET_SIZE || length > NASA_MAX_PACKET_SIZE) {
        return false;
    }

    size_t size = (data[1] << 8) | data[2];
    if (size + 2 != length) {
        return false;
    }

    if (data[length - 1] != NASA_END_BYTE) {
        return false;
    }

    // Verify CRC
    uint16_t crcActual = NASAPacket::crc16(data, 3, size - 4);
    uint16_t crcExpected = (data[length - 3] << 8) | data[length - 2];
    if (crcExpected != crcActual) {
        return false;
    }

    size_t cursor = 3;

    // Decode source address
    _sourceAddress.decode(data, cursor);
    cursor += NASAAddress::SIZE;

    // Decode destination address
    _destinationAddress.decode(data, cursor);
    cursor += NASAAddress::SIZE;

    // Decode command
    _command.decode(data, cursor);
    cursor += NASACommand::SIZE;

    // Get message capacity
    uint8_t capacity = data[cursor];
    cursor++;

    // Clear existing messages and decode new ones
    clearMessages();
    ensureCapacity(capacity);

    for (uint8_t i = 0; i < capacity; i++) {
        if (cursor >= length - 3) {
            return false;
        }
        NASAMessageSet message;
        size_t limit = (length - 3) - cursor;
        size_t msgSize = message.decode(data, cursor, limit);
        if (msgSize == 0) {
            return false;
        }
        _messages[_messageCount++] = message;
        cursor += msgSize;
    }

    return true;
}

size_t NASAPacket::encode(uint8_t* buffer, size_t maxLength) {
    if (maxLength < NASA_MIN_PACKET_SIZE) {
        return 0;
    }

    size_t cursor = 3;  // Skip start byte and size for now

    // Encode source address
    _sourceAddress.encode(buffer, cursor);
    cursor += NASAAddress::SIZE;

    // Encode destination address
    _destinationAddress.encode(buffer, cursor);
    cursor += NASAAddress::SIZE;

    // Encode command
    _command.encode(buffer, cursor);
    cursor += NASACommand::SIZE;

    // Encode message count
    buffer[cursor] = _messageCount;
    cursor++;

    // Encode messages
    for (size_t i = 0; i < _messageCount; i++) {
        size_t expectedMsgSize = _messages[i].getSize();
        if (cursor + expectedMsgSize + 3 > maxLength) {
            return 0;
        }
        size_t msgSize = _messages[i].encode(buffer, cursor);
        cursor += msgSize;
    }

    // Calculate size (without start byte and size field)
    size_t size = cursor + 3 - 2;  // +3 for CRC and end byte, -2 for start and size fields

    // Set start byte and size
    buffer[0] = NASA_START_BYTE;
    buffer[1] = (size >> 8) & 0xFF;
    buffer[2] = size & 0xFF;

    // Calculate and set CRC
    uint16_t crc = NASAPacket::crc16(buffer, 3, size - 4);
    buffer[cursor] = (crc >> 8) & 0xFF;
    buffer[cursor + 1] = crc & 0xFF;
    cursor += 2;

    // Set end byte
    buffer[cursor] = NASA_END_BYTE;
    cursor++;

    return cursor;
}

bool NASAPacket::addMessage(const NASAMessageSet& message) {
    if (_messageCount >= 255) {
        return false;
    }

    ensureCapacity(_messageCount + 1);
    _messages[_messageCount++] = message;
    return true;
}

const NASAMessageSet* NASAPacket::getMessage(size_t index) const {
    if (index >= _messageCount) {
        return nullptr;
    }
    return &_messages[index];
}

void NASAPacket::clearMessages() {
    _messageCount = 0;
}

bool NASAPacket::hasMessage(uint16_t messageNumber) const {
    for (size_t i = 0; i < _messageCount; i++) {
        if (_messages[i].getMessageNumber() == messageNumber) {
            return true;
        }
    }
    return false;
}

uint32_t NASAPacket::getMessageValue(uint16_t messageNumber, uint32_t defaultValue) const {
    for (size_t i = 0; i < _messageCount; i++) {
        if (_messages[i].getMessageNumber() == messageNumber) {
            return _messages[i].getValue();
        }
    }
    return defaultValue;
}

void NASAPacket::ensureCapacity(size_t capacity) {
    if (capacity <= _messageCapacity) {
        return;
    }

    size_t newCapacity = capacity + 8;  // Allocate extra space
    NASAMessageSet* newMessages = new NASAMessageSet[newCapacity];

    if (_messages) {
        for (size_t i = 0; i < _messageCount; i++) {
            newMessages[i] = _messages[i];
        }
        delete[] _messages;
    }

    _messages = newMessages;
    _messageCapacity = newCapacity;
}