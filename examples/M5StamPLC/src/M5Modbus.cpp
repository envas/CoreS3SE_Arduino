/*
 * Example for the "CWT-THxxS Series RS485 temperature and humidity sensor"
 * with Modbus address = 2
 *
 * Created by Robert Carnecky on 24.09.2025.
 */

#include "M5Modbus.hpp"


/**
 *
 * @param baud
 */
M5Modbus::M5Modbus(uint16_t baud) {
    _baudrate = baud;
    _MB = new ModbusClientRTU(REDE_PIN);
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
    // Provide onData handler function
    _MB->onDataHandler([this](ModbusMessage rsp, uint32_t token) {
        this->handleData(rsp, token);
    });
    // Provide onError handler function
    _MB->onErrorHandler([this](Error err, uint32_t token) {
        this->handleError(err, token);
    });
    // Set message timeout to 2000ms
    _MB->setTimeout(2000);
    // Setup Serial1 for ModbusRTU
    RTUutils::prepareHardwareSerial(Serial1);
     // Setup Serial 1 parameters. For Serial1 (and Serial2) we can use any pins
    Serial1.begin(_baudrate, SERIAL_8N1, RX_PIN, TX_PIN);
    // Start ModbusRTU background task
    _MB->begin(Serial1);
}

/**
 * Send request
 */
Error M5Modbus::addRequest(uint32_t token, uint8_t address, FunctionCode func, uint16_t reg, uint16_t num) {
    return _MB->addRequest(token, address, func, reg, num);

}
