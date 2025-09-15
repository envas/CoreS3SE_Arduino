//
// Created by Robert Carnecky on 05.09.2025.
//

#include <M5Unified.hpp>
#include <RadioLib.h>

#include "utils.h"


/**
 *
 * @param message
 * @param state
 */
void debug(const __FlashStringHelper* message, int state) {
    M5.Display.print("ERR: ");
    M5.Display.println(message);
    M5.Display.print(state2text(state));
    M5.Display.print(" (");
    M5.Display.print(state);
    M5.Display.println(")");
}

/**
 * Helper function to display a byte array
 */
void array_dump(uint8_t* buffer, uint16_t len) {
    for (uint16_t c = 0; c < len; c++) {
        char b = buffer[c];
        if (b < 0x10) {
            M5.Display.print('0');
        }
        M5.Display.print(b, HEX);
        M5.Display.print(" ");
    }
    M5.Display.println();
}

/**
 * Result code to text - these are error codes that can be raised when using LoRaWAN
 * however, RadioLib has many more - see https://jgromes.github.io/RadioLib/group__status__codes.html for a complete list
 */
String state2text(const int16_t result) {
    switch (result) {
        case RADIOLIB_ERR_NONE:
            return "ERR_NONE";
        case RADIOLIB_ERR_CHIP_NOT_FOUND:
            return "ERR_CHIP_NOT_FOUND";
        case RADIOLIB_ERR_PACKET_TOO_LONG:
            return "ERR_PACKET_TOO_LONG";
        case RADIOLIB_ERR_RX_TIMEOUT:
            return "ERR_RX_TIMEOUT";
        case RADIOLIB_ERR_MIC_MISMATCH:
            return "ERR_MIC_MISMATCH";
        case RADIOLIB_ERR_INVALID_BANDWIDTH:
            return "ERR_INVALID_BANDWIDTH";
        case RADIOLIB_ERR_INVALID_SPREADING_FACTOR:
            return "ERR_INVALID_SPREADING_FACTOR";
        case RADIOLIB_ERR_INVALID_CODING_RATE:
            return "ERR_INVALID_CODING_RATE";
        case RADIOLIB_ERR_INVALID_FREQUENCY:
            return "ERR_INVALID_FREQUENCY";
        case RADIOLIB_ERR_INVALID_OUTPUT_POWER:
            return "ERR_INVALID_OUTPUT_POWER";
        case RADIOLIB_ERR_NETWORK_NOT_JOINED:
            return "RADIOLIB_ERR_NETWORK_NOT_JOINED";
        case RADIOLIB_ERR_DOWNLINK_MALFORMED:
            return "RADIOLIB_ERR_DOWNLINK_MALFORMED";
        case RADIOLIB_ERR_INVALID_REVISION:
            return "RADIOLIB_ERR_INVALID_REVISION";
        case RADIOLIB_ERR_INVALID_PORT:
            return "RADIOLIB_ERR_INVALID_PORT";
        case RADIOLIB_ERR_NO_RX_WINDOW:
            return "RADIOLIB_ERR_NO_RX_WINDOW";
        case RADIOLIB_ERR_INVALID_CID:
            return "RADIOLIB_ERR_INVALID_CID";
        case RADIOLIB_ERR_UPLINK_UNAVAILABLE:
            return "RADIOLIB_ERR_UPLINK_UNAVAILABLE";
        case RADIOLIB_ERR_COMMAND_QUEUE_FULL:
            return "RADIOLIB_ERR_COMMAND_QUEUE_FULL";
        case RADIOLIB_ERR_COMMAND_QUEUE_ITEM_NOT_FOUND:
            return "RADIOLIB_ERR_COMMAND_QUEUE_ITEM_NOT_FOUND";
        case RADIOLIB_ERR_JOIN_NONCE_INVALID:
            return "RADIOLIB_ERR_JOIN_NONCE_INVALID";
        case RADIOLIB_ERR_DWELL_TIME_EXCEEDED:
            return "RADIOLIB_ERR_DWELL_TIME_EXCEEDED";
        case RADIOLIB_ERR_CHECKSUM_MISMATCH:
            return "RADIOLIB_ERR_CHECKSUM_MISMATCH";
        case RADIOLIB_ERR_NO_JOIN_ACCEPT:
            return "RADIOLIB_ERR_NO_JOIN_ACCEPT";
        case RADIOLIB_LORAWAN_SESSION_RESTORED:
            return "RADIOLIB_LORAWAN_SESSION_RESTORED";
        case RADIOLIB_LORAWAN_NEW_SESSION:
            return "RADIOLIB_LORAWAN_NEW_SESSION";
        case RADIOLIB_ERR_NONCES_DISCARDED:
            return "RADIOLIB_ERR_NONCES_DISCARDED";
        case RADIOLIB_ERR_SESSION_DISCARDED:
            return "RADIOLIB_ERR_SESSION_DISCARDED";
    }
    return "See https://jgromes.github.io/RadioLib/group__status__codes.html";
}
