//
// Created by Robert Carnecky on 22.09.2025.
//

#include <M5Modbus.hpp>
#include <Sensor.hpp>
#include "Timespec.h"
#include <thread>

void print_now() {
    struct timespec ts;
    timespec_now(&ts);
    TIMESPEC_BUFFER str;
    timespec_to_str(str, &ts);
    Serial.printf("%s\n", str);
}

Sensor::Sensor(uint8_t id, M5Modbus* modbus, uint8_t addr, String name, String description) {
    _id = id;
    if (name.length() == 0) {
        name = "S-" + String(_id);
    }
    if (description.length() == 0) {
        description = "Sensor #" + String(_id);
    }
    _name           = name;
    _description    = description;
    _modbus_address = addr;
    _modbus         = modbus;
    _temperature    = 0;
    _humidity       = 0;

    _last_poll_time = timespec_now_to_msec();
}

void Sensor::poll() {
    int64_t now = timespec_now_to_msec();
    if (_last_poll_time + POLL_INTERVAL <= now) {
        std::thread t(&Sensor::doPoll, this);
        t.detach();
        Serial.printf("Polling sensor %s\n", getDescription().c_str());
        Serial.printf("  Temperature: %3.1f\n", getTemperatureF());
        Serial.printf("  Humidity: %3.1f\n", getHumidityF());
        _last_poll_time = timespec_now_to_msec();
    }
}

uint16_t Sensor::getHumidity() {
    return _humidity;
}

int16_t Sensor::getTemperature() {
    return _temperature;
}

void Sensor::doPoll() {
    ModbusMessage req = createModbusMessage();
    ModbusMessage rsp = _modbus->syncRequest(req, _id);
    parseModbusMessage(rsp);
}

float Sensor::getHumidityF() {
    return _humidity / 10.0f;
}

float Sensor::getTemperatureF() {
    return _temperature / 10.0f;
}

void Sensor::setId(uint8_t id) {
    _id = id;
}

uint8_t Sensor::getId() {
    return _id;
}

void Sensor::setName(String name) {
    _name = name;
}

void Sensor::setHumidity(uint16_t humidity) {
    _humidity = humidity;
}

void Sensor::setTemperature(int16_t temperature) {
    _temperature = temperature;
}

String Sensor::getName() {
    if (_name.length() == 0) {
        return "S-" + String(_id);
    }
    return _name;
}

void Sensor::setDescription(String description) {
    _description = description;
}

String Sensor::getDescription() {
    if (_description.length() == 0) {
        return "Sensor #" + String(_id);
    }
    return _description;
}

void Sensor::setModbusAddress(uint8_t addr) {
    _modbus_address = addr;
}

uint8_t Sensor::getModbusAddress() {
    return _modbus_address;
}

ModbusMessage Sensor::createModbusMessage() {
    return ModbusMessage( _modbus_address, READ_HOLD_REGISTER, 0x0000, 0x02);
}

void Sensor::parseModbusMessage(ModbusMessage msg) {
    msg.get(3, _humidity);
    msg.get(5, _temperature);
}
