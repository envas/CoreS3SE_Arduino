//
// Created by Robert Carnecky on 22.09.2025.
//

#include <Arduino.h>
#include <M5Unified.hpp>
#include "Sensor.hpp"
#include "ModbusClientRTU.h"

#ifndef M5STACK_MODBUS_H
#define M5STACK_MODBUS_H

// values for M5StamPLC
#define RX_PIN   GPIO_NUM_39
#define TX_PIN   GPIO_NUM_0
#define REDE_PIN GPIO_NUM_46

class M5Modbus {
    ModbusClientRTU* _MB;

protected:
    HardwareSerial _serial;
    uint16_t       _rx_pin;
    uint16_t       _tx_pin;
    uint16_t       _rede_pin;
    uint32_t       _baudrate;

public:
    M5Modbus(HardwareSerial& serial, uint16_t baud = 9600);
    ~M5Modbus();

    void begin();
    Error addRequest(uint32_t token, uint8_t address, FunctionCode func, uint16_t reg, uint16_t num);
    void handleData(ModbusMessage response, uint32_t token);
    void handleError(Error error, uint32_t token);
};

#endif // M5STACK_MODBUS_H
