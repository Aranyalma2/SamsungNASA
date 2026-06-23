#include "SamsungNASA.h"

#include <esp_log.h>

SamsungNASA::SamsungNASA()
    : _serial(nullptr),
      _hwSerial(nullptr),
      _reDePin(-1),
      _packetHandler(nullptr),
      _packetNumber(0),
      _receiveTaskHandle(nullptr),
      _sendMutex(nullptr),
      _receiveBufferPos(0),
      _lastByteTime(0) {
}

SamsungNASA::~SamsungNASA() {
    end();
}

bool SamsungNASA::begin(
    HardwareSerial* serial,
    int8_t rxPin,
    int8_t txPin,
    int8_t reDePin,
    uint8_t deviceClass,
    uint8_t deviceChannel,
    uint8_t deviceAddress) {
    if (serial == nullptr) {
        return false;
    }
    _hwSerial = serial;
    _serial = serial;
    _reDePin = reDePin;
    _deviceAddress = NASAAddress(deviceClass, deviceChannel, deviceAddress);

    // Configure RE/DE pin
    if (_reDePin >= 0) {
        pinMode(_reDePin, OUTPUT);
        setReceiveMode();
    }

    // Initialize serial (9600 baud, Even parity, 1 stop bit)
    _hwSerial->begin(9600, SERIAL_8E1, rxPin, txPin);

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

bool SamsungNASA::begin(Stream* serial, uint8_t deviceClass, uint8_t deviceChannel, uint8_t deviceAddress) {
    if (serial == nullptr) {
        return false;
    }
    _serial = serial;
    _hwSerial = nullptr;
    _deviceAddress = NASAAddress(deviceClass, deviceChannel, deviceAddress);

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

    if (_hwSerial != nullptr) {
        _hwSerial->end();
    }
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
    if (_serial != nullptr) {
        _serial->write(buffer, length);
        _serial->flush();
    }

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
    size_t totalSize = 0;

    ESP_LOGI("NASA_RX", "NASA receive task started");

    while (true) {
        bool readAny = false;
        if (instance->_serial != nullptr && instance->_serial->available()) {
            TickType_t now = xTaskGetTickCount();
            while (instance->_serial->available()) {
                uint8_t byte = instance->_serial->read();
                instance->_lastByteTime = now;
                readAny = true;

                if (instance->_receiveBufferPos == 0) {
                    if (byte != NASA_START_BYTE) {
                        continue; // Skip bytes before a valid start byte
                    }
                }

                // Add to buffer
                if (instance->_receiveBufferPos < NASA_MAX_PACKET_SIZE) {
                    instance->_receiveBuffer[instance->_receiveBufferPos++] = byte;

                    // Check if we have a complete packet header
                    if (instance->_receiveBufferPos == 3) {
                        size_t expectedSize = (instance->_receiveBuffer[1] << 8) | instance->_receiveBuffer[2];
                        totalSize = expectedSize + 2;

                        if (totalSize < NASA_MIN_PACKET_SIZE || totalSize > NASA_MAX_PACKET_SIZE) {
                            // Invalid size, reset buffer to find next start byte
                            instance->_receiveBufferPos = 0;
                            totalSize = 0;
                        }
                    } else if (instance->_receiveBufferPos >= 3 && instance->_receiveBufferPos == totalSize) {
                        instance->processReceiveBuffer();
                        instance->_receiveBufferPos = 0;
                        totalSize = 0;
                    }
                } else {
                    // Buffer overflow, reset
                    instance->_receiveBufferPos = 0;
                    totalSize = 0;
                }
            }
        }

        if (!readAny) {
            // No data was read. Check for timeout.
            if (instance->_receiveBufferPos > 0) {
                if (xTaskGetTickCount() - instance->_lastByteTime > pdMS_TO_TICKS(500)) {
                    ESP_LOGD("NASA_RX", "Receive buffer timeout");
                    instance->_receiveBufferPos = 0;
                    totalSize = 0;
                }
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}

void SamsungNASA::processReceiveBuffer() {
    ESP_LOGD("NASA_RX", "NASA message received");
    if (_receivePacket.decode(_receiveBuffer, _receiveBufferPos)) {
        if (_packetHandler != nullptr) {
            _packetHandler(_receivePacket);
        }
    } else {
        ESP_LOGD("NASA_RX", "NASA message decoding failed");
    }
}