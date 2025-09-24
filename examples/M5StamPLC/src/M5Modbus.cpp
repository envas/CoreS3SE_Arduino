/*
 * Example for the "CWT-THxxS Series RS485 temperature and humidity sensor"
 * with Modbus address = 2
 *
 * Created by Robert Carnecky on 24.09.2025.
 */

#include "M5Modbus.hpp"

M5Modbus::M5Modbus(HardwareSerial& serial, uint16_t baud) {
    _serial   = serial;
    _baudrate = baud;
    _MB       = new ModbusClientRTU(REDE_PIN);
}

M5Modbus::~M5Modbus() {
    delete _MB;
}


/**
 * Received data handler.
 *
 * @param response
 * @param token
 */
void M5Modbus::handleData(ModbusMessage response, uint32_t token) {
    Serial.println("Modbus data received");
}

/**
 * Received error response handler
 * @param error
 * @param token
 */
void handleError(Error error, uint32_t token) {
    Serial.println("Modbus error received");
}

/**
 * Initialization happens here
 */
void M5Modbus::begin() {
    // Set up Serial2 connected to Modbus RTU
    RTUutils::prepareHardwareSerial(_serial);
    _serial.begin(_baudrate, SERIAL_8N1, RX_PIN, TX_PIN);

    // Set up ModbusRTU client.
    // - provide onData handler function
    _MB->onDataHandler([this](ModbusMessage rsp, uint32_t token) {
        this->handleData(rsp, token);
    });
    // - provide onError handler function
    _MB->onErrorHandler([this](Error err, uint32_t token) {
        this->handleError(err, token);
    });
    // Set message timeout to 2000ms
    _MB->setTimeout(2000);
    // Start ModbusRTU background task
    _MB->begin(_serial);
}

/**
 * Send request
 */
Error M5Modbus::addRequest(uint32_t token, uint8_t address, FunctionCode func, uint16_t reg, uint16_t num) {
    return _MB->addRequest(token, address, func, reg, num);

}
