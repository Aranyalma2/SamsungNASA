#ifndef NASA_PROTOCOL_H
#define NASA_PROTOCOL_H

#include <Arduino.h>

// Protocol constants
#define NASA_START_BYTE 0x32
#define NASA_END_BYTE 0x34
#define NASA_MIN_PACKET_SIZE 16
#define NASA_MAX_PACKET_SIZE 1500

// Address Classes
namespace AddressClass {
    enum : uint8_t {
        Outdoor = 0x10,
        HTU = 0x11,
        Indoor = 0x20,
        ERV = 0x30,
        Diffuser = 0x35,
        MCU = 0x38,
        RMC = 0x40,
        WiredRemote = 0x50,
        PIM = 0x58,
        SIM = 0x59,
        Peak = 0x5A,
        PowerDivider = 0x5B,
        OnOffController = 0x60,
        WiFiKit = 0x62,
        MIM = 0x63,
        CentralController = 0x65,
        DMS = 0x6A,
        JIGTester = 0x80,
        BroadcastSelfLayer = 0xB0,
        BroadcastControlLayer = 0xB1,
        BroadcastSetLayer = 0xB2,
        BroadcastControlAndSetLayer = 0xB3,
        BroadcastModuleLayer = 0xB4,
        BroadcastCSM = 0xB7,
        BroadcastLocalLayer = 0xB8,
        BroadcastCSML = 0xBF,
        Undefined = 0xFF
    };
}

// Packet Types
namespace PacketType {
    enum : uint8_t {
        StandBy = 0,
        Normal = 1,
        Gathering = 2,
        Install = 3,
        Download = 4
    };
}

// Data Types
namespace DataType {
    enum : uint8_t {
        Undefined = 0,
        Read = 1,
        Write = 2,
        Request = 3,
        Notification = 4,
        Response = 5,
        Ack = 6,
        Nack = 7
    };
}

// Message Set Types
namespace MessageSetType {
    enum : uint8_t {
        Enum = 0,
        Variable = 1,
        LongVariable = 2,
        Structure = 3
    };
}

// Common Message Numbers
namespace MessageNumber {
    enum : uint16_t {
        ENUM_IN_OPERATION_POWER = 0x4000,
        ENUM_IN_OPERATION_MODE = 0x4001,
        ENUM_IN_FAN_MODE = 0x4006,
        ENUM_IN_FAN_MODE_REAL = 0x4007,
        ENUM_IN_LOUVER_HL_SWING = 0x4011,
        ENUM_IN_STATE_HUMIDITY_PERCENT = 0x4038,
        ENUM_IN_ALT_MODE = 0x4060,
        ENUM_IN_WATER_HEATER_POWER = 0x4065,
        ENUM_IN_WATER_HEATER_MODE = 0x4066,
        ENUM_IN_LOUVER_LR_SWING = 0x407E,
        ENUM_IN_OPERATION_AUTOMATIC_CLEANING = 0x4111,
        VAR_IN_TEMP_TARGET_F = 0x4201,
        VAR_IN_TEMP_ROOM_F = 0x4203,
        VAR_IN_TEMP_EVA_IN_F = 0x4205,
        VAR_IN_TEMP_EVA_OUT_F = 0x4206,
        VAR_IN_TEMP_WATER_HEATER_TARGET_F = 0x4235,
        VAR_IN_TEMP_WATER_TANK_F = 0x4237,
        VAR_IN_TEMP_WATER_OUTLET_TARGET_F = 0x4247,
        VAR_OUT_SENSOR_AIROUT = 0x8204,
        VAR_OUT_SENSOR_CT1 = 0x8217,
        VAR_OUT_ERROR_CODE = 0x8235,
        LVAR_OUT_CONTROL_WATTMETER_1W_1MIN_SUM = 0x8413,
        LVAR_OUT_CONTROL_WATTMETER_ALL_UNIT_ACCUM = 0x8414,
        LVAR_NM_OUT_SENSOR_VOLTAGE = 0x24FC
    };
}

#endif // NASA_PROTOCOL_H