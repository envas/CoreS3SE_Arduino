//
// Created by Robert Carnecky on 22.09.2025.
//

#include <M5Modbus.hpp>
#include <Sensor.hpp>
#include "Timespec.h"

Sensor::Sensor(uint8_t id, M5Modbus* modbus, uint8_t addr, String name, String description) {
    _id             = id;
    _name           = name;
    _description    = description;
    _modbus_address = addr;
    _modbus         = modbus;
}

void Sensor::poll() {
    int64_t now = timespec_now_to_msec();
    if (_last_poll_time + 1 * 1000 >= now) {
        Error err = _modbus->addRequest(_id, _modbus_address, READ_HOLD_REGISTER, 0x0000, 0x02);
        if (err != SUCCESS) {
            ModbusError e(err);
        }
    }
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

void Sensor::setModbus(M5Modbus* modbus) {
    _modbus = modbus;
}

M5Modbus* Sensor::getModbus() {
    return _modbus;
}
