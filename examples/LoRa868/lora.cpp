#include <Arduino.h>
#include <M5GFX.h>
#include <M5Unified.h>
#include <RadioLib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LORA_BW 500.0
#define LORA_SF 7
#define LORA_CR 8

#define LORA_FREQ         868.0
#define LORA_SYNC_WORD    0x34
#define LORA_TX_POWER     22
#define LORA_PREAMBLE_LEN 20

#define CONFIG_MISO_GPIO GPIO_NUM_35
#define CONFIG_MOSI_GPIO GPIO_NUM_37
#define CONFIG_SCLK_GPIO GPIO_NUM_36

#define CONFIG_LORA_NSS  GPIO_NUM_0
#define CONFIG_LORA_BUSY GPIO_NUM_2
#define CONFIG_LORA_RST  GPIO_NUM_7
#define CONFIG_LORA_IRQ  GPIO_NUM_10

SX1276* radio = NULL;

// save transmission states between loops
int transmissionState = RADIOLIB_ERR_NONE;

// flag to indicate transmission or reception state
bool transmitFlag = false;

volatile bool operationDone = false;

ICACHE_RAM_ATTR void setFlag(void) {
    // we sent or received a packet, set the flag
    operationDone = true;
}

void lora_init() {
    radio = new SX1276(new Module(CONFIG_LORA_NSS, // NSS
                                  CONFIG_LORA_IRQ, // DIO1
                                  CONFIG_LORA_RST, // RST
                                  CONFIG_LORA_BUSY)); // BUSY

    M5.Display.println("LoRa init ...");

    //   RADIOLIB_ERR_NONE
    int state = radio->begin(
        LORA_FREQ,
        LORA_BW,
        LORA_SF,
        LORA_CR,
        LORA_SYNC_WORD,
        LORA_TX_POWER,
        LORA_PREAMBLE_LEN,
        0);

    M5.Display.println("Radio begin OK");

    radio->setDio1Action(setFlag, GPIO_MODE_INPUT_OUTPUT);

    M5.Display.println("DIO1 action set OK");

    Serial.print(F("Starting to listen ... "));

    state = radio->startReceive();

    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("success!"));
        M5.Display.println("LoRa init success");
        M5.Display.println("Touch to send packet");
    } else {
        Serial.print(F("failed, code "));
        M5.Display.println("LoRa init failed");
        Serial.println(state);
        while (true) {
            delay(10);
        }
    }
}

void setup() {
    M5.begin();
    M5.Display.setTextColor(YELLOW);
    M5.Display.setFont(&fonts::FreeSansBold9pt7b);
    M5.Display.setTextScroll(true);

    Serial.begin(115200);

    lora_init();
}

int msg_count = 0;
String tx_payload;
String rx_payload;

void loop() {
    M5.update();
    auto t = M5.Touch.getDetail();

    if (t.wasClicked() || M5.BtnA.wasClicked()) {
        // send another one
        msg_count++;
        Serial.print(F("Sending another packet ... "));

        tx_payload = "Hello LoRa Count: " + String(msg_count);
        transmissionState = radio->transmit(tx_payload);
        M5.Display.println("Sending packet!");
        M5.Display.println(tx_payload);
        transmitFlag = true;
    }

    // check if the previous operation finished
    if (operationDone) {
        // reset flag
        operationDone = false;
        if (transmitFlag) {
            transmitFlag = false;
            int state = radio->startReceive();
        } else {
            int state = radio->readData(rx_payload);
            Serial.println("lora_rx_flag");
            Serial.println(state);
            if (state == RADIOLIB_ERR_NONE) {
                // packet was successfully received
                Serial.println(F("Received packet!"));

                // print data of the packet
                Serial.print(F("Data:\t\t"));
                Serial.println(rx_payload);

                // print RSSI (Received Signal Strength Indicator)
                Serial.print(F("RSSI:\t\t"));
                Serial.print(radio->getRSSI());
                Serial.println(F(" dBm"));

                // print packet length
                Serial.print(F("Length:\t\t"));
                Serial.print(radio->getPacketLength());

                // print SNR (Signal-to-Noise Ratio)
                Serial.print(F("SNR:\t\t"));
                Serial.print(radio->getSNR());
                Serial.println(F(" dB"));

                // display on screen
                M5.Display.println("Received packet!");
                M5.Display.println("Data:");
                M5.Display.println(rx_payload);
                M5.Display.print("RSSI:");
                M5.Display.print(radio->getRSSI());
                M5.Display.println(" dBm");
                M5.Display.print("SNR:");
                M5.Display.print(radio->getSNR());
                M5.Display.println(" dB");
            }
        }
    }
}
