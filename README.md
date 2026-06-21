# SamsungNASA - Samsung NASA Protocol Library for ESP32

An Arduino library for communicating with Samsung HVAC systems using the NASA protocol over RS485. Optimized exclusively for ESP32 with FreeRTOS async packet handling.
For AVR architecture check this: [SamsungNASA-AVR](https://github.com/Aranyalma2/SamsungNASA-AVR)

## Features

- **Full NASA Protocol Implementation** - Complete encoding/decoding based on relativly new version of the protocol
- **ESP32 Optimized** - Uses FreeRTOS for async packet handling
- **RS485 Support** - Built-in RE/DE pin control for RS485 transceivers
- **CRC16 Validation** - Automatic packet integrity checking
- **Callback Architecture** - Easy-to-use event-driven packet handling
- **All Message Types** - Supports Enum, Variable, LongVariable, and Structure messages
- **Flexible Device Types** - Configure as any NASA protocol device (default: Undefined)

## Hardware Requirements

- **ESP32** development board (any variant)
- **RS485 Transceiver** module (e.g. MAX3485, or similar)
- Connection to **Samsung HVAC** NASA bus

## Wiring

```
RS485 Module          ESP32
─────────────────────────────
RO (Receiver Out) ->   GPIO16 (RX)
DI (Driver In)    ->   GPIO17 (TX)
RE (Recv Enable)  ->   GPIO4
DE (Driver Enable)->   GPIO4 (connect RE and DE together)
VCC               ->   3.3V
GND               ->   GND

RS485 A/B         ->   Samsung F1/F2, F3/F4, R1/R2
```

## Protocol Details

### Packet Structure

```
┌────────┬──────┬──────┬────────┬──────────┬─────────┬──────────┬────────┬─────┬────────┐
│ Start  │ Size │ Size │ Source │ Dest.    │ Command │ Msg      │ Data   │ CRC │ End    │
│ (0x32) │ (HI) │ (LO) │ (3B)   │ (3B)     │ (3B)    │ Count    │ (var)  │ (2B)│ (0x34) │
└────────┴──────┴──────┴────────┴──────────┴─────────┴──────────┴────────┴─────┴────────┘
```

### Message Types

- **Enum** (Type 0): 1-byte value (e.g., ON/OFF, mode selection)
- **Variable** (Type 1): 2-byte value (e.g., temperature, fan speed)
- **LongVariable** (Type 2): 4-byte value (e.g., power consumption)
- **Structure** (Type 3): Variable length structured data

### Temperature Encoding

Temperatures are encoded as `value * 10`:

- 24.0°C -> 240
- 18.5°C -> 185
- 30.0°C -> 300

## Troubleshooting

### No packets received

1. Check wiring (especially A/B polarity)
2. Verify baud rate (usually 9600)
3. Ensure RS485 module is powered correctly
4. Check RE/DE pin is connected and configured

### CRC errors

1. Check for electrical noise on RS485 bus
2. Ensure proper grounding
3. Use twisted pair cable for longer runs
4. Add termination resistors if needed (120Ω)

## Advanced Usage

### Custom Device Types

```cpp
// Act as a central controller
nasa.begin(&Serial1, 16, 17, 4, AddressClass::CentralController, 0, 1);
```

### Packet Filtering

```cpp
void onPacketReceived(const NASAPacket& packet) {
  // Only process packets from indoor units
  if (packet.getSourceAddress().getClass() != AddressClass::Indoor) {
    return;
  }

  // Process packet...
}
```

### Broadcasting

```cpp
// Send to all indoor units
NASAAddress broadcast(AddressClass::BroadcastControlLayer, 0, 0);
NASAPacket packet = nasa.createPacket(broadcast, DataType_Notification);
// Add messages and send...
```

## License

MIT License - See LICENSE file for details

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## Credits

Based on the Samsung NASA protocol reverse engineering efforts and few surfaced documents.

Thanks to the contributors of [ESPHome Samsung HVAC Integration](https://github.com/omerfaruk-aran/esphome_samsung_hvac_bus) for interpreting some of the protocol addresses.

## Disclaimer

This library is intended for educational use in home and educational applications and is not affiliated with or endorsed by Samsung. Use at your own risk. Always comply with local regulations regarding modifications to HVAC systems.
