#include "SamsungNASA.h"

SamsungNASA::SamsungNASA(HardwareSerial& serial)
    : _serial(serial),
      _reDePin(-1),
      _packetHandler(nullptr),
      _packetNumber(0),
      _receiveTaskHandle(nullptr),
      _sendMutex(nullptr),
      _receiveBufferPos(0) {
}

SamsungNASA::~SamsungNASA() {
    end();
}

bool SamsungNASA::begin(uint32_t baudRate,
                        int8_t rxPin,
                        int8_t txPin,
                        int8_t reDePin,
                        uint8_t deviceClass,
                        uint8_t deviceChannel,
                        uint8_t deviceAddress) {
    _reDePin = reDePin;
    _deviceAddress = NASAAddress(deviceClass, deviceChannel, deviceAddress);

    // Configure RE/DE pin
    if (_reDePin >= 0) {
        pinMode(_reDePin, OUTPUT);
        setReceiveMode();
    }

    // Initialize serial
    _serial.begin(baudRate, SERIAL_8E1, rxPin, txPin);

    // Create mutex
    _sendMutex = xSemaphoreCreateMutex();
    if (_sendMutex == nullptr) {
        return false;
    }

    // Create receive task
    BaseType_t result = xTaskCreate(
        receiveTask,
        "NASA_RX",
        4096,
        this,
        5,
        &_receiveTaskHandle);

    if (result != pdPASS) {
        vSemaphoreDelete(_sendMutex);
        _sendMutex = nullptr;
        return false;
    }

    return true;
}

void SamsungNASA::end() {
    // Delete receive task
    if (_receiveTaskHandle != nullptr) {
        vTaskDelete(_receiveTaskHandle);
        _receiveTaskHandle = nullptr;
    }

    // Delete mutex
    if (_sendMutex != nullptr) {
        vSemaphoreDelete(_sendMutex);
        _sendMutex = nullptr;
    }

    _serial.end();
}

void SamsungNASA::setPacketHandler(PacketHandler handler) {
    _packetHandler = handler;
}

bool SamsungNASA::sendPacket(const NASAPacket& packet) {
    if (_sendMutex == nullptr) {
        return false;
    }

    // Take mutex
    if (xSemaphoreTake(_sendMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        return false;
    }

    uint8_t buffer[NASA_MAX_PACKET_SIZE];
    size_t length = const_cast<NASAPacket&>(packet).encode(buffer, NASA_MAX_PACKET_SIZE);

    if (length == 0) {
        xSemaphoreGive(_sendMutex);
        return false;
    }

    // Set transmit mode
    setTransmitMode();

    // Send data
    _serial.write(buffer, length);
    _serial.flush();

    // Set receive mode
    setReceiveMode();

    // Give mutex
    xSemaphoreGive(_sendMutex);

    return true;
}

bool SamsungNASA::sendPacket(const NASAAddress& destination,
                             uint8_t dataType,
                             uint8_t packetType) {
    NASAPacket packet = createPacket(destination, dataType);
    packet.getCommand().setPacketType(packetType);
    return sendPacket(packet);
}

NASAPacket SamsungNASA::createPacket(const NASAAddress& destination, uint8_t dataType) {
    NASAPacket packet;
    packet.getSourceAddress() = _deviceAddress;
    packet.getDestinationAddress() = destination;
    packet.getCommand().setDataType(dataType);
    packet.getCommand().setPacketType(PacketType::Normal);
    packet.getCommand().setPacketNumber(_packetNumber++);
    return packet;
}

void SamsungNASA::setTransmitMode() {
    if (_reDePin >= 0) {
        digitalWrite(_reDePin, HIGH);
        delayMicroseconds(50);  // Small delay for hardware to switch
    }
}

void SamsungNASA::setReceiveMode() {
    if (_reDePin >= 0) {
        digitalWrite(_reDePin, LOW);
    }
}

void SamsungNASA::receiveTask(void* parameter) {
    SamsungNASA* instance = static_cast<SamsungNASA*>(parameter);

    while (true) {
        if (instance->_serial.available()) {
            uint8_t byte = instance->_serial.read();

            // Check for start byte
            if (byte == NASA_START_BYTE) {
                instance->_receiveBufferPos = 0;
            }

            // Add to buffer
            if (instance->_receiveBufferPos < NASA_MAX_PACKET_SIZE) {
                instance->_receiveBuffer[instance->_receiveBufferPos++] = byte;

                // Check if we have a complete packet
                if (instance->_receiveBufferPos >= 3) {
                    size_t expectedSize = (instance->_receiveBuffer[1] << 8) | instance->_receiveBuffer[2];
                    size_t totalSize = expectedSize + 2;

                    if (instance->_receiveBufferPos >= totalSize) {
                        instance->processReceiveBuffer();
                        instance->_receiveBufferPos = 0;
                    }
                }
            } else {
                // Buffer overflow, reset
                instance->_receiveBufferPos = 0;
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}

void SamsungNASA::processReceiveBuffer() {
    NASAPacket packet;

    if (packet.decode(_receiveBuffer, _receiveBufferPos)) {
        if (_packetHandler != nullptr) {
            _packetHandler(packet);
        }
    }
}