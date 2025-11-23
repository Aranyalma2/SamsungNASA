#ifndef SAMSUNG_NASA_H
#define SAMSUNG_NASA_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "NASAProtocol.h"
#include "NASAPacket.h"

typedef void (*PacketHandler)(const NASAPacket& packet);

enum BUS_TYPE : uint32_t {
    F1_F2 = SERIAL_8N1,
    R1_R2 = SERIAL_8E1
};

class SamsungNASA {
public:
    SamsungNASA(HardwareSerial& serial = Serial2);
    ~SamsungNASA();

    // Initialize the library
    bool begin(
        uint32_t baudRate = 9600,
        BUS_TYPE busType = F1_F2,
        int8_t rxPin = 16, 
        int8_t txPin = 17, 
        int8_t reDePin = 4,
        uint8_t deviceClass = AddressClass::Undefined,
        uint8_t deviceChannel = 0,
        uint8_t deviceAddress = 0);
    
    // Stop the library
    void end();
    
    // Set packet handler callback
    void setPacketHandler(PacketHandler handler);
    
    // Send a packet
    bool sendPacket(const NASAPacket& packet);
    
    // Helper: Send a simple packet with messages
    bool sendPacket(const NASAAddress& destination,
                   DataType dataType,
                   PacketType packetType = PacketType::Normal);
    
    // Get device address
    const NASAAddress& getDeviceAddress() const { return _deviceAddress; }
    
    // Packet building helper
    NASAPacket createPacket(const NASAAddress& destination, DataType dataType);
    
private:
    HardwareSerial& _serial;
    int8_t _reDePin;
    NASAAddress _deviceAddress;
    PacketHandler _packetHandler;
    uint8_t _packetNumber;
    
    // FreeRTOS
    TaskHandle_t _receiveTaskHandle;
    SemaphoreHandle_t _sendMutex;
    
    // Receive buffer
    uint8_t _receiveBuffer[NASA_MAX_PACKET_SIZE];
    size_t _receiveBufferPos;
    
    // Task functions
    static void receiveTask(void* parameter);
    void processReceiveBuffer();
    
    // RS485 control
    void setTransmitMode();
    void setReceiveMode();
};

#endif // SAMSUNG_NASA_H