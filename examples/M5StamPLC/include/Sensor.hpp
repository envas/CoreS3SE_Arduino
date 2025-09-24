//
// Created by Robert Carnecky on 22.09.2025.
//

#include <Arduino.h>


#ifndef M5STACK_SENSOR_H
#define M5STACK_SENSOR_H

class M5Modbus;

class Sensor {
private:
    uint64_t  _last_poll_time;
    M5Modbus* _modbus;

protected:
    uint8_t _id;
    String  _name;
    String  _description;

    uint8_t  _modbus_address;

public:
    // constructor with default values
    Sensor(uint8_t id = 0, M5Modbus* modbus = nullptr, uint8_t addr = 1, String name = "", String description = "");

    // method for sensor value(s) update
    void poll();

    // setters/getters
    uint8_t   getId();
    String    getName();
    String    getDescription();
    uint8_t   getModbusAddress();
    M5Modbus* getModbus();


    void setName(String name);
    void setDescription(String description);
    void setId(uint8_t id);
    void setModbusAddress(uint8_t addr);
    void setModbus(M5Modbus* modbus);
};


#endif // M5STACK_SENSOR_H
