#include "NASAProtocol.h"

uint16_t nasa_crc16(const uint8_t* data, size_t startIndex, size_t length) {
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