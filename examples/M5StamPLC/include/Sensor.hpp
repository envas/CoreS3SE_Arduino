//
// Created by Robert Carnecky on 22.09.2025.
//

#include <Arduino.h>

#ifndef M5STACK_SENSOR_H
#define M5STACK_SENSOR_H

// polling interval 5 seconds
#define POLL_INTERVAL 5000

class M5Modbus;

class Sensor {
protected:
    uint8_t   _id;
    String    _name;
    String    _description;
    M5Modbus* _modbus;
    uint8_t   _modbus_address;
    uint64_t  _last_poll_time;

    // sensor values
    int16_t  _temperature;
    uint16_t _humidity;

    // this method executes the poll in the separate thread
    void doPoll();

public:
    // constructors
    Sensor();
    explicit Sensor(uint8_t id, M5Modbus* modbus, uint8_t addr, String name, String description);

    // method for sensor value(s) update
    void poll();

    // Modbus messages - make it virtual in the real world
    ModbusMessage createModbusMessage();
    void          parseModbusMessage(ModbusMessage msg);

    // setters/getters
    uint8_t  getId();
    String   getName();
    String   getDescription();
    uint8_t  getModbusAddress();
    int16_t  getTemperature();
    uint16_t getHumidity();
    float    getTemperatureF();
    float    getHumidityF();

    void setName(String name);
    void setDescription(String description);
    void setId(uint8_t id);
    void setModbusAddress(uint8_t addr);
    void setTemperature(int16_t temp);
    void setHumidity(uint16_t hum);
};

#endif // M5STACK_SENSOR_H
