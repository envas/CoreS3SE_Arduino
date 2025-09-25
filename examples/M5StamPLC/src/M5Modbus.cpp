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
M5Modbus::M5Modbus(HardwareSerial* serial, uint16_t baud) {

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
void M5Modbus::handleError(Error error, uint32_t token) {
    Serial.println("Modbus error received");
}

/**
 * Initialization
 */
void M5Modbus::begin() {

    _MB->onDataHandler([this](ModbusMessage rsp, uint32_t token) {
        this->handleData(rsp, token);
    });

    _MB->onErrorHandler([this](Error err, uint32_t token) {
        this->handleError(err, token);
    });

    _MB->setTimeout(1000);

    RTUutils::prepareHardwareSerial(*_serial);;

    _serial->begin(_baudrate, SERIAL_8N1, RX_PIN, TX_PIN);

    _MB->begin(*_serial);
}

/**
 * Send request - non blocking
 */
Error M5Modbus::addRequest(ModbusMessage msg, uint32_t token) {
    return _MB->addRequest(msg, token);
}

/**
 * Send request - blocking
 * @param msg
 * @param token
 * @return
 */
ModbusMessage M5Modbus::syncRequest(ModbusMessage msg, uint32_t token) {
    return _MB->syncRequest(msg, token);
}
