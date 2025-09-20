//
// Created by Robert Carnecky on 19.09.2025.
//

#include <Relay.hpp>
#include <M5StamPLC.h>

// default relay
Relay::Relay() {
    _id = 0;
}

// relay 0..3
Relay::Relay(uint8_t i) {
    _id = i;
}

// relay 0..3 with name and description
Relay::Relay(uint8_t i, String n, String d) {
    _id          = i;
    _name        = n;
    _description = d;
}

// actions
void Relay::switchOn() {
    M5StamPLC.writePlcRelay(_id, true);
}

Relay::~Relay() {
    switchOff();
}

void Relay::switchOff() {
    M5StamPLC.writePlcRelay(_id, false);
}

void Relay::toggle() {
    isOn() ? switchOff() : switchOn();
}

bool Relay::isOn() {
    return M5StamPLC.readPlcRelay(_id);
}

bool Relay::isOff() {
    return !isOn();
}
