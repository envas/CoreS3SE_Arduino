//
// Created by Robert Carnecky on 19.09.2025.
//

#ifndef M5STACK_RELAY_H
#define M5STACK_RELAY_H

#include <Arduino.h>

class Relay {

protected:
    uint8_t  _id;
    String   _name;
    String   _description;

public:
    Relay();
    ~Relay();

    // constructors
    Relay(uint8_t id);
    Relay(uint8_t id, String name, String description);

    // actions
    void switchOn();
    void switchOff();
    void toggle();
    bool isOn();
    bool isOff();
};


#endif //M5STACK_RELAY_H
