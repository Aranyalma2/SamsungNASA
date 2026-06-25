#ifndef SAMSUNG_NASA_H
#define SAMSUNG_NASA_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <Stream.h>

#include "NASAPacket.h"
#include "NASAProtocol.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#ifndef NASA_TASK_STACK_SIZE
#define NASA_TASK_STACK_SIZE 2048
#endif

typedef void (*PacketHandler)(const NASAPacket& packet);

class SamsungNASA {
   public:
    SamsungNASA();
    ~SamsungNASA();

    // Initialize the library
    bool begin(
        HardwareSerial* serial,
        int8_t rxPin = 16,
        int8_t txPin = 17,
        int8_t reDePin = 4,
        uint8_t deviceClass = AddressClass::Undefined,
        uint8_t deviceChannel = 0,
        uint8_t deviceAddress = 0);

    // Alternative begin with outside Serial initialization
    bool begin(
        Stream* serial,
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
                    uint8_t dataType,
                    uint8_t packetType = PacketType::Normal);

    // Get device address
    const NASAAddress& getDeviceAddress() const { return _deviceAddress; }

    // Packet building helper
    NASAPacket createPacket(const NASAAddress& destination, uint8_t dataType);

   private:
    Stream* _serial;
    HardwareSerial* _hwSerial;
    int8_t _reDePin = -1;
    NASAAddress _deviceAddress;
    PacketHandler _packetHandler;
    uint8_t _packetNumber;

    // FreeRTOS
    TaskHandle_t _receiveTaskHandle;
    SemaphoreHandle_t _sendMutex;

    // Receive buffer
    uint8_t _receiveBuffer[NASA_MAX_PACKET_SIZE];
    size_t _receiveBufferPos;
    NASAPacket _receivePacket;
    TickType_t _lastByteTime;

    // Task functions
    static void receiveTask(void* parameter);
    void processReceiveBuffer();

    // RS485 control
    void setTransmitMode();
    void setReceiveMode();
};

#endif  // SAMSUNG_NASA_H