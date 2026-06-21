#include "NASAMessageSet.h"

NASAMessageSet::NASAMessageSet(uint16_t messageNumber)
    : _messageNumber(messageNumber), _value(0), _size(2) {
    updateType();
}

void NASAMessageSet::setMessageNumber(uint16_t value) {
    _messageNumber = value;
    updateType();
}

void NASAMessageSet::updateType() {
    _type = (_messageNumber & 0x0600) >> 9;
    switch (_type) {
        case MessageSetType::Enum:
            _size = 3;
            break;
        case MessageSetType::Variable:
            _size = 4;
            break;
        case MessageSetType::LongVariable:
            _size = 6;
            break;
        case MessageSetType::Structure:
        default:
            _size = 2;
            break;
    }
}

size_t NASAMessageSet::decode(const uint8_t* data, size_t index, size_t limit) {
    if (limit < 2) {
        _size = 0;
        return 0;
    }
    _messageNumber = (data[index] << 8) | data[index + 1];
    updateType();

    switch (_type) {
        case MessageSetType::Enum:
            if (limit < 3) {
                _size = 0;
                return 0;
            }
            _value = data[index + 2];
            _size = 3;
            break;

        case MessageSetType::Variable:
            if (limit < 4) {
                _size = 0;
                return 0;
            }
            _value = (data[index + 2] << 8) | data[index + 3];
            _size = 4;
            break;

        case MessageSetType::LongVariable:
            if (limit < 6) {
                _size = 0;
                return 0;
            }
            _value = ((uint32_t)data[index + 2] << 24) |
                     ((uint32_t)data[index + 3] << 16) |
                     ((uint32_t)data[index + 4] << 8) |
                     data[index + 5];
            _size = 6;
            break;

        case MessageSetType::Structure:
            _size = 2;
            break;
    }

    return _size;
}

size_t NASAMessageSet::encode(uint8_t* data, size_t index) const {
    data[index] = (_messageNumber >> 8) & 0xFF;
    data[index + 1] = _messageNumber & 0xFF;

    switch (_type) {
        case MessageSetType::Enum:
            data[index + 2] = _value & 0xFF;
            return 3;

        case MessageSetType::Variable:
            data[index + 2] = (_value >> 8) & 0xFF;
            data[index + 3] = _value & 0xFF;
            return 4;

        case MessageSetType::LongVariable:
            data[index + 2] = (_value >> 24) & 0xFF;
            data[index + 3] = (_value >> 16) & 0xFF;
            data[index + 4] = (_value >> 8) & 0xFF;
            data[index + 5] = _value & 0xFF;
            return 6;

        case MessageSetType::Structure:
            return 2;
    }

    return 2;
}