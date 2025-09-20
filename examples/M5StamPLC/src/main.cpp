//
// Created by Robert Carnecky on 15.09.2025.
//

#include <Arduino.h>
#include <M5StamPLC.h>
#include <Relay.hpp>

Relay relay;

void setup() {
    M5StamPLC.begin();

    Serial.begin(115200);
    Serial.println("Setup finished");
}

void loop() {
    delay(2000);
    relay.toggle();
    Serial.printf("Relay toggled, state = %s\n", relay.isOn() ? "ON" : "OFF");
}
