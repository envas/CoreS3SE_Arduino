//
// Created by Robert Carnecky on 15.09.2025.
//

#include <Arduino.h>
#include <M5StamPLC.h>
#include <Relay.hpp>

#include <M5Modbus.hpp>
#include <Sensor.hpp>

void setup() {
    M5StamPLC.begin();

    M5Modbus* modbus = new M5Modbus(19200);
    Sensor sensor = Sensor(0, modbus, 2);

    Serial.begin(115200);
    Serial.println("Setup finished");
}

void loop() {
    delay(2000);
}
