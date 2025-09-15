//
// Created by Robert Carnecky on 15.09.2025.
//

#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>

void deepsleep(uint32_t seconds) {

    // enter deep sleep  for `seconds`
    // M5.Power.deepSleep(seconds * 1000UL * 1000UL, true);
    M5.Power.deepSleep(seconds * 1000UL * 1000UL); // time to sleep in microseconds

    // DO NOT USE THE ESP32-IDF API WHEN ON BATTERY!
    // ESP32 api switches off everything except RTC domain including GPIO pin managing the battery MOSFET switch
    // The easiest way is to call ESP.deepSleep (ESP class is an ESP32 wrapper). Alternative could be to move
    // the battery GPIO to the RTCIO (which GPIO?)
    //
    // ESP32 API:
    // esp_sleep_enable_timer_wakeup(seconds * 1000UL * 1000UL); // function uses uS
    // esp_deep_sleep_start();

    // if this appears on the display, we didn't go to sleep!
    // so take defensive action so we don't continually uplink
    Serial.println("\nSleep failed, delay of 5 minutes & restart");
    delay(5UL * 60UL * 1000UL);
    ESP.restart();
}



void setup() {

    Serial.begin(115200);

    auto cfg = M5.config();
    cfg.output_power = false;
    cfg.pmic_button = false;
    cfg.internal_imu = false;
    cfg.internal_mic = false;
    cfg.internal_spk = false;

    M5.begin(cfg);
    M5.Power.begin();

    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);

    M5.Imu.sleep();


    // disable Bluetooth
    btStop();

    M5.update();

    deepsleep(15);

}



void loop() {

}