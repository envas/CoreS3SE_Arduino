//
// Created by Robert Carnecky on 05.09.2025.
//

#include <Arduino.h>
#include <Preferences.h>
#include <RadioLib.h>
#include <M5Unified.hpp>

#include "main.h"
#include "utils.h"
#include "lora.h"

uint64_t joinEUI  = RADIOLIB_LORAWAN_JOIN_EUI;
uint64_t devEUI   = RADIOLIB_LORAWAN_DEV_EUI;
uint8_t  appKey[] = {RADIOLIB_LORAWAN_APP_KEY};
uint8_t  nwkKey[] = {RADIOLIB_LORAWAN_NWK_KEY};


SX1276      radio = new Module(CONFIG_LORA_NSS, CONFIG_LORA_IRQ, CONFIG_LORA_RST);
LoRaWANNode node(&radio, &EU868, 0);

// Probuzení z hlubokého spánku je jako resetování celého procesoru, s výjimkou toho,
// že subdoména RTC zůstává aktivní. V praxi se rozlisuje probuzení z hlubokého spánku
// podle důvodu resetu. Paměť RTC zůstává neporušená.
//
// Pro ulozeni do paměti RTC se promenne definuje jako RTC_DATA_ATTR.
RTC_DATA_ATTR uint16_t bootCount                      = 0;
RTC_DATA_ATTR uint16_t bootCountSinceUnsuccessfulJoin = 0;
RTC_DATA_ATTR uint8_t  lw_session[RADIOLIB_LORAWAN_SESSION_BUF_SIZE];

// In-Memory nonces
uint8_t nonces[RADIOLIB_LORAWAN_NONCES_BUF_SIZE];

// EEPROM store pro permanentno schovani nonces
Preferences store;

/**
 * Saves the nonces to persistent storage.
 *
 * This function retrieves the nonces buffer using the LoRaWANNode instance
 * and stores it in the Preferences storage with a specific key. The storage
 * operation includes initiating the storage namespace, saving the nonces
 * data, and closing the storage session.
 */
void lora_save_nonces() {
    store.begin("radiolib");
    uint8_t* nonces = node.getBufferNonces();
    store.putBytes("nonces", nonces, RADIOLIB_LORAWAN_NONCES_BUF_SIZE);
    store.end();
}

/**
 * If exists, restores the nonces from the persistent storage and updates
 * the node's nonces buffer.
 *
 * @return
 */
int16_t lora_restore_nonces() {
    int16_t state = RADIOLIB_ERR_NONCES_DISCARDED;
    store.begin("radiolib");
    if (store.isKey("nonces")) {
        uint8_t nonces[RADIOLIB_LORAWAN_NONCES_BUF_SIZE];
        store.getBytes("nonces", nonces, RADIOLIB_LORAWAN_NONCES_BUF_SIZE);
        state = node.setBufferNonces(nonces);
    }
    store.end();

    if (state != RADIOLIB_ERR_NONE) {
        debug(F("restore nonces"), state);
    }
    return state;
}

/**
 * Saves the current LoRaWAN session data to a session buffer.
 *
 * This function retrieves the session data buffer using the LoRaWANNode instance
 * and copies the session data into the local session buffer for persistent use.
 * The operation ensures that the session state is available for further processing
 * or storage.
 */
void lora_save_session() {
    uint8_t* session = node.getBufferSession();
    memcpy(lw_session, session, RADIOLIB_LORAWAN_SESSION_BUF_SIZE);
}

/**
 * Tries to restore the session from the persistent RTC memory.
 *
 * @return state RADIOLIB_LORAWAN_SESSION_RESTORED or some error
 */
int16_t lora_restore_session() {
    int16_t state = RADIOLIB_ERR_NONCES_DISCARDED;

    state = lora_restore_nonces();
    if (state != RADIOLIB_ERR_NONE) {
        return state;
    }

    state = node.setBufferSession(lw_session);
    if (state != RADIOLIB_ERR_NONE) {
        if (bootCount > 1) {
            // error reporting from boot #2 up
            debug(F("restore session"), state);
        }

        return state;
    }

    state = node.activateOTAA();
    if (state != RADIOLIB_LORAWAN_SESSION_RESTORED) {
        debug(F("activate OTAA"), state);
        return state;
    }

    return state;
}


/**
 * Attempt to join LoRaWAN network. Saves the nonces in the EEPROM for
 * the next attempt usage.
 *
 * @return state
 */
uint16_t lora_join() {
    int16_t state = RADIOLIB_ERR_UNKNOWN;

    if (node.isActivated()) {
        return RADIOLIB_LORAWAN_SESSION_RESTORED;
    }

    state = RADIOLIB_ERR_NETWORK_NOT_JOINED;

    // restore the nonces
    state = lora_restore_nonces();
    if (state == RADIOLIB_ERR_NONCES_DISCARDED) {
        // the first attempt from the device, no nonces exists
        // probably required an action on the TTN (reset nonces
        // counter or setup to ignore nonces)
        debug(F("restore nonces"), state);
    }

    while (state != RADIOLIB_LORAWAN_NEW_SESSION) {
        state = node.activateOTAA();

        // save the join counters (nonces) to the EEPROM
        lora_save_nonces();

        // ooops
        if (state != RADIOLIB_LORAWAN_NEW_SESSION) {
            M5.Display.clear();
            M5.Display.setCursor(0, 0);
            debug(F("join"), state);

            /*
             * In LoRaWAN, there isn't a single fixed wait time between join attempts; rather,
             * it depends on the device's firmware and the overall network conditions, often
             * incorporating an exponential backoff strategy. A device first listens for a
             * Join-Accept message in the RX1 window after a short, fixed delay
             * (like JOIN_ACCEPT_DELAY1 of 5 seconds), then potentially in RX2. If no
             * Join-Accept is received, the device must wait, respecting regional duty cycle
             * limits and often implementing a backoff mechanism that increases the delay after
             * each failed attempt until a successful join occurs or a maximum wait time is reached.
             */
            uint32_t sleep_seconds = min((bootCountSinceUnsuccessfulJoin++ + 1UL) * 60UL, 3UL * 60UL);
            M5.Display.print("Next join in ");
            M5.Display.print(sleep_seconds);
            M5.Display.println(" sec.");
            vTaskDelay(sleep_seconds * 1000UL / portTICK_PERIOD_MS);
        }
    }

    return state;
}

/**
 * Initialize node internal structures, no restoere session or join attempt here
 *
 * @return state
 */
int16_t lora_node_init() {
    int16_t state = RADIOLIB_ERR_UNKNOWN;

    // LoRa modem initialization method. Must be called at least once to initialize the module.
    state = radio.begin();
    if (state != RADIOLIB_ERR_NONE) {
        debug(F("initialise radio"), state);
        return (state);
    }

    // Set the device credentials and activation configuration
    state = node.beginOTAA(joinEUI, devEUI, nwkKey, appKey);
    if (state != RADIOLIB_ERR_NONE) {
        debug(F("initialize OTAA node"), state);
    }

    return state;
}

/**
 * LoRaWAN activation. This call occurs after the device reboots or after the
 * wakeup from the deep sleep.
 *
 * @return status
 */
int16_t lora_activate() {
    int16_t state = RADIOLIB_ERR_UNKNOWN;

    if (node.isActivated()) {
        return RADIOLIB_LORAWAN_SESSION_RESTORED;
    }

    state = lora_node_init();
    if (state != RADIOLIB_ERR_NONE) {
        return state;
    }

    state = lora_restore_session();
    if (state == RADIOLIB_LORAWAN_SESSION_RESTORED) {
        M5.Display.println("LoRaWAN session restored");
        return state;
    }

    state = lora_join();
    if (state == RADIOLIB_LORAWAN_NEW_SESSION) {
        M5.Display.println("LoRaWAN join success");
    }

    return state;
}

/**
 * Sends uplink payload and (if handler defined and downlink data available)
 * calls a downlink payload handler.
 *
 * @param data_up      data buffer with uplink payload
 * @param data_up_len  uplink payload data length (in bytes)
 * @param callback     downlink data handler
 *
 * @return state
 */
int16_t lora_send_receive(uint8_t* data_up, size_t data_up_len, void (*callback)(uint8_t*, size_t)) {
    int16_t        state         = RADIOLIB_ERR_UNKNOWN;
    size_t         data_down_len = 0;
    uint8_t        data_down[RADIOLIB_LORAWAN_MAX_DOWNLINK_SIZE + 1];
    LoRaWANEvent_t event_up;
    LoRaWANEvent_t event_down;

    // perform an uplink
    state = node.sendReceive(data_up, data_up_len, 1, data_down, &data_down_len, false, &event_up, &event_down);

    if (state < RADIOLIB_ERR_NONE) {
        debug(F("data upload"), state);
    } else if (state > RADIOLIB_ERR_NONE) {
        // received downlink
        M5.Display.println("Received downlink");
        if (callback != NULL) {
            callback(data_down, data_down_len);
        }
    }

    return state;
}

 void lora_sleep() {
     radio.sleep();
 }

