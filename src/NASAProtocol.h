#ifndef NASA_PROTOCOL_H
#define NASA_PROTOCOL_H

#include <Arduino.h>

// Protocol constants
#define NASA_START_BYTE 0x32
#define NASA_END_BYTE 0x34
#define NASA_MIN_PACKET_SIZE 16
#define NASA_MAX_PACKET_SIZE 1500

// Address Classes
enum AddressClass : uint8_t {
    AddressClass_Outdoor = 0x10,
    AddressClass_HTU = 0x11,
    AddressClass_Indoor = 0x20,
    AddressClass_ERV = 0x30,
    AddressClass_Diffuser = 0x35,
    AddressClass_MCU = 0x38,
    AddressClass_RMC = 0x40,
    AddressClass_WiredRemote = 0x50,
    AddressClass_PIM = 0x58,
    AddressClass_SIM = 0x59,
    AddressClass_Peak = 0x5A,
    AddressClass_PowerDivider = 0x5B,
    AddressClass_OnOffController = 0x60,
    AddressClass_WiFiKit = 0x62,
    AddressClass_CentralController = 0x65,
    AddressClass_DMS = 0x6A,
    AddressClass_JIGTester = 0x80,
    AddressClass_BroadcastSelfLayer = 0xB0,
    AddressClass_BroadcastControlLayer = 0xB1,
    AddressClass_BroadcastSetLayer = 0xB2,
    AddressClass_BroadcastControlAndSetLayer = 0xB3,
    AddressClass_BroadcastModuleLayer = 0xB4,
    AddressClass_BroadcastCSM = 0xB7,
    AddressClass_BroadcastLocalLayer = 0xB8,
    AddressClass_BroadcastCSML = 0xBF,
    AddressClass_Undefined = 0xFF
};

// Packet Types
enum PacketType : uint8_t {
    PacketType_StandBy = 0,
    PacketType_Normal = 1,
    PacketType_Gathering = 2,
    PacketType_Install = 3,
    PacketType_Download = 4
};

// Data Types
enum DataType : uint8_t {
    DataType_Undefined = 0,
    DataType_Read = 1,
    DataType_Write = 2,
    DataType_Request = 3,
    DataType_Notification = 4,
    DataType_Response = 5,
    DataType_Ack = 6,
    DataType_Nack = 7
};

// Message Set Types
enum MessageSetType : uint8_t {
    MessageSetType_Enum = 0,
    MessageSetType_Variable = 1,
    MessageSetType_LongVariable = 2,
    MessageSetType_Structure = 3
};

// CRC16 calculation
uint16_t nasa_crc16(const uint8_t* data, size_t startIndex, size_t length);

#endif // NASA_PROTOCOL_H