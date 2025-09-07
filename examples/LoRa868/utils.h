//
// Created by Robert Carnecky on 05.09.2025.
//

#ifndef CORES3SE_ARDUINO_UTILS_H
#define CORES3SE_ARDUINO_UTILS_H

#include <Arduino.h>

void   debug(const __FlashStringHelper* message, int state);
void   array_dump(uint8_t* buffer, uint16_t len);
String state2text(const int16_t result);
float  calculate_altitude(const float, const float);

#endif //CORES3SE_ARDUINO_UTILS_H
