/*
 * Samsung NASA Protocol - Basic Sender Example
 * 
 * This example demonstrates how to send NASA protocol packets
 * to HVAC devices.
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
 * - RS485 A                    -> NASA A
 * - RS485 B                    -> NASA B
 */

#include <SamsungNASA.h>

// Create NASA protocol instance
SamsungNASA nasa(Serial2);

// Target device address (Broadcast to self-layer)
NASAAddress broadcastAddr(AddressClass_BroadcastSelfLayer, 0x0F, 0xFF);

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  
  Serial.println("\n====================================");
  Serial.println("Samsung NASA Protocol - Basic Sender");
  Serial.println("====================================\n");
  
  // Initialize NASA protocol as an Outdoor device
  if (!nasa.begin(9600, F1_F2, 16, 17, 4, AddressClass_Undefined, 0x0F, 0x00)) {
    Serial.println("ERROR: Failed to initialize NASA protocol!");
    while (1) {
      delay(1000);
    }
  }
  
  Serial.println("NASA protocol initialized successfully!\n");
}

void loop() {
  // Example: Broadcast a write to some registers to all devices.
  Serial.println("Broadcasting registers (enum + variable) to all self-layer devices...");

  // Create a broadcast packet. Using DataType_Notification to indicate info update.
  NASAPacket packet = nasa.createPacket(broadcastAddr, DataType_Notification);

  // Example 1: Enum message (power ON)
  NASAMessageSet msgPower(MSG_ENUM_IN_OPERATION_POWER);
  msgPower.setValue(1); // 1 = ON
  packet.addMessage(msgPower);

  // Send the broadcast packet
  if (nasa.sendPacket(packet)) {
    Serial.println("Broadcast packet sent successfully!");
  } else {
    Serial.println("ERROR: Failed to send broadcast packet!");
  }

  Serial.println();
  delay(5000); // Wait 5 seconds before next broadcast
}