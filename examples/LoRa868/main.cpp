/*
  RadioLib LoRaWAN Starter Example

  ! Please refer to the included notes to get started !

  This example joins a LoRaWAN network and will send
  uplink packets. Before you start, you will have to
  register your device at https://www.thethingsnetwork.org/
  After your device is registered, you can run this example.
  The device will join the network and start uploading data.

  Running this examples REQUIRES you to check "Resets DevNonces"
  on your LoRaWAN dashboard. Refer to the network's
  documentation on how to do this.

  For default module settings, see the wiki page
  https://github.com/jgromes/RadioLib/wiki/Default-configuration

  For full API reference, see the GitHub Pages
  https://jgromes.github.io/RadioLib/

  For LoRaWAN details, see the wiki page
  https://github.com/jgromes/RadioLib/wiki/LoRaWAN

*/

#include <Arduino.h>
#include <M5GFX.h>
#include <M5Unified.h>
#include "config.h"

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(YELLOW);
    M5.Display.setFont(&fonts::FreeMono9pt7b);
    M5.Display.setTextScroll(false);

    Serial.begin(115200);
    SPI.begin();

    M5.Display.clear();

    while (!Serial) { // Give time to switch to the serial monitor
        delay(500);
    }
    M5.Display.println("\nSetup ... ");

    M5.Display.print("Init radio ");

    int16_t state = radio.begin();
    debug(state != RADIOLIB_ERR_NONE, F("Initialise radio failed"), state, true);
    M5.Display.println("... OK");

    // Setup the OTAA session information
    M5.Display.print("Init node  ");
    state = node.beginOTAA(joinEUI, devEUI, nwkKey, appKey);
    debug(state != RADIOLIB_ERR_NONE, F("Initialise node failed"), state, true);
    M5.Display.println("... OK");

    M5.Display.print("Join       ");
    state = node.activateOTAA();
    debug(state != RADIOLIB_LORAWAN_NEW_SESSION, F("Join failed"), state, true);
    M5.Display.println("... OK");

    M5.Display.println("Ready!\n");

    delay(5000); // Wait for the user to read the messages (if any
    M5.Display.clear();
}

void loop() {
    M5.Display.fillRect(0, 0, 320, 240, TFT_RED);
    M5.Display.drawString("SENDING UPLINK", 10, 10);

    // This is the place to gather the sensor inputs;
    // Instead of reading any real sensor, we just generate some random numbers as an example
    uint8_t  value1 = radio.random(100);
    uint16_t value2 = radio.random(2000);

    // Build payload byte array
    uint8_t uplinkPayload[3];
    uplinkPayload[0] = value1;
    uplinkPayload[1] = highByte(value2); // See notes for high/lowByte functions
    uplinkPayload[2] = lowByte(value2);

    // Perform an uplink
    int16_t state = node.sendReceive(uplinkPayload, sizeof(uplinkPayload));
    debug(state < RADIOLIB_ERR_NONE, F("Error in sendReceive"), state, false);

    // Check if a downlink was received
    // (state 0 = no downlink, state 1/2 = downlink in window Rx1/Rx2)
    if (state > 0) {
        M5.Display.drawString("Received a downlink", 10, 40);
    } else {
        M5.Display.drawString("No downlink received", 10, 40);
    }

    // Wait until next uplink - observing legal & TTN FUP constraints
    delay(uplinkIntervalSeconds * 1000UL); // delay needs milliseconds
}
