//
// Created by Robert Carnecky on 15.09.2025.
//


#include "Main.hpp"

Sensor*   sensor;
M5Modbus* modbus;

void setup() {
    // Setup PLC
    M5StamPLC.begin();

    // Setup modbus RTU client on Serial1
    modbus = new M5Modbus(&Serial1, 9600);
    modbus->begin();

    // Setup sensor
    sensor = new Sensor(0, modbus, 2, "", "");

    Serial.begin(115200);
    Serial.println("Setup finished");
}

void loop() {
    sensor->poll();
}
