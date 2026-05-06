/*
 * Samsung NASA Protocol - Basic Receiver Example
 *
 *  Author: Aranyalma2
 *
 * This example demonstrates how to receive and decode NASA protocol packets
 * from HVAC devices.
 *
 * Hardware Requirements:
 * - ESP32 board
 * - RS485 transceiver module (e.g., MAX3485)
 *
 * Wiring:
 * - RS485 RO (Receiver Output) -> ESP32 GPIO16 (RX)
 * - RS485 DI (Driver Input)    -> ESP32 GPIO17 (TX)
 * - RS485 RE (Receiver Enable) -> ESP32 GPIO4
 * - RS485 DE (Driver Enable)   -> ESP32 GPIO4
 * - RS485 A                    -> NASA F1 or R1
 * - RS485 B                    -> NASA F2 or R2
 *
 * Note: RE and DE pins are typically connected together
 */

#include <SamsungNASA.h>

// Create NASA protocol instance using Serial2
SamsungNASA nasa(Serial2);

// Packet handler callback function
void onPacketReceived(const NASAPacket& packet) {
    Serial.println("====================================");
    Serial.println("NASA Packet Received!");
    Serial.println("====================================");

    // Print source address
    const NASAAddress& src = packet.getSourceAddress();
    Serial.printf("Source: Class=0x%02X, Channel=%d, Address=%d\n",
                  src.getClass(), src.getChannel(), src.getAddress());

    // Print destination address
    const NASAAddress& dst = packet.getDestinationAddress();
    Serial.printf("Destination: Class=0x%02X, Channel=%d, Address=%d\n",
                  dst.getClass(), dst.getChannel(), dst.getAddress());

    // Print command info
    const NASACommand& cmd = packet.getCommand();
    Serial.printf("Data Type: %d, Packet Type: %d, Packet #: %d\n",
                  cmd.getDataType(), cmd.getPacketType(), cmd.getPacketNumber());

    // Print messages
    size_t msgCount = packet.getMessageCount();
    Serial.printf("\nMessages (%d):\n", msgCount);

    for (size_t i = 0; i < msgCount; i++) {
        const NASAMessageSet* msg = packet.getMessage(i);
        if (msg) {
            Serial.printf("  %d. Msg#: 0x%04X, Type: %d, Value: %u (0x%08X)\n",
                          i + 1,
                          msg->getMessageNumber(),
                          msg->getType(),
                          msg->getValue(),
                          msg->getValue());

            // Decode specific known messages
            switch (msg->getMessageNumber()) {
                case MessageNumber::ENUM_IN_OPERATION_POWER:
                    Serial.printf("     -> Power: %s\n", msg->getValue() ? "ON" : "OFF");
                    break;

                case MessageNumber::ENUM_IN_OPERATION_MODE: {
                    const char* modes[] = {"Auto", "Cool", "Dry", "Fan", "Heat"};
                    uint8_t mode = msg->getValue();
                    if (mode < 5) {
                        Serial.printf("     -> Mode: %s\n", modes[mode]);
                    }
                } break;

                case MessageNumber::ENUM_IN_FAN_MODE:
                case MessageNumber::ENUM_IN_FAN_MODE_REAL: {
                    const char* fans[] = {"Auto", "Low", "Mid", "High", "Turbo"};
                    uint8_t fan = msg->getValue();
                    if (fan < 5) {
                        Serial.printf("     -> Fan: %s\n", fans[fan]);
                    }
                } break;

                case MessageNumber::VAR_IN_TEMP_TARGET_F:
                    Serial.printf("     -> Target Temperature: %.1f°C\n", msg->getValue() / 10.0);
                    break;

                case MessageNumber::VAR_IN_TEMP_ROOM_F:
                    Serial.printf("     -> Room Temperature: %.1f°C\n", msg->getValue() / 10.0);
                    break;

                case MessageNumber::VAR_OUT_ERROR_CODE:
                    Serial.printf("     -> Error Code: 0x%04X\n", msg->getValue());
                    break;
            }
        }
    }

    Serial.println("====================================\n");
}

void setup() {
    // Initialize serial for debug output
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }

    Serial.println("\n\n====================================");
    Serial.println("Samsung NASA Protocol - Basic Receiver");
    Serial.println("====================================\n");

    // Initialize NASA protocol
    // Parameters: baudRate, rxPin, txPin, reDePin, deviceClass, deviceChannel, deviceAddress
    if (!nasa.begin(9600, 16, 17, 4, AddressClass::Undefined, 0x0F, 0x01)) {
        Serial.println("ERROR: Failed to initialize NASA protocol!");
        while (1) {
            delay(1000);
        }
    }

    Serial.println("NASA protocol initialized successfully!");
    Serial.println("Listening for packets...\n");

    // Set packet handler
    nasa.setPacketHandler(onPacketReceived);
}

void loop() {
    // Everything is handled by FreeRTOS task in the background
    // You can add your own code here
    delay(100);
}