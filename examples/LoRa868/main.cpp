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
#include <BMP280.h>
#include <SHT4X.h>
#include <RadioLib.h>

#include "main.h"
#include "lora.h"
#include "utils.h"

extern RTC_DATA_ATTR uint16_t bootCount;

// environmental sensor
SHT4X  sht4;
BMP280 bmp;

/**
 * Report wakeup with the reason. Abbreviated version from the Arduino-ESP32 package, see
 * https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/deepsleep.html
 * for the complete set of options
 */
void print_wakeup_reason() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
        M5.Display.println(F("Wake from timer"));
    } else {
        M5.Display.println(F("Wake up not from timer!"));
        M5.Display.print(F("Wake up reason: "));
        M5.Display.println(wakeup_reason);
    }

    M5.Display.print(F("Boot count: "));
    M5.Display.println(++bootCount); // increment before printing
}

/**
 * Puts the device into the lowest power deep-sleep mode
 */
void deepsleep(uint32_t seconds) {
    M5.Display.println("Going to sleep ...");

    // give the user a chance to read the display
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    // turn off the display before entering deep sleep (ESP.deepSleep does not do it)
    M5.Display.sleep();
    M5.Display.powerSave(true);

    // enter deep sleep  for `seconds`
    M5.update();
    ESP.deepSleep(seconds * 1000UL * 1000UL); // time to sleep in microseconds

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
    M5.Display.println("\nSleep failed, delay of 5 minutes & restart");
    delay(5UL * 60UL * 1000UL);
    ESP.restart();
}

/**
 * Handler for downlink payload
 * 
 * @param data  buffer with downlink data
 * @param len   downlink data length
 */
void downlink_handler(uint8_t* data, size_t len) {
    M5.Display.println("Downlink handler data:");
    array_dump(data, len);
}


/**
 * Initializes hardware and software components, sends LoRaWAN uplink payload,
 * and transitions the system to deep sleep mode.
 *
 * This method:
 * - Configures and initializes M5 core components.
 * - Sets up serial communication and SPI.
 * - Displays text settings on the screen.
 * - Prints the wakeup reason after a reset or wakeup from deep sleep.
 * - Activates the LoRaWAN session, determines if a new session is created
 *   or restored, and prepares battery-related data to send via LoRaWAN.
 * - Sends uplink data through LoRaWAN and updates the session to maintain connectivity.
 * - Configures the ESP32 deep sleep timer and initiates the deep sleep mode.
 *
 * Preconditions:
 * - The method uses global variables and hardware components such as the M5 stack and LoRaWAN module.
 * - RTC_DATA_ATTR variable `bootCount` must be defined for tracking the number of boots.
 * - Proper dependencies and configurations for the M5 stack, LoRaWAN, and ESP32 deep sleep API must be in place.
 *
 * LoRaWAN payload structure:
 * - Battery charging status (1 byte).
 * - Battery voltage in millivolts (2 bytes, MSB first).
 * - Battery percentage level (1 byte).
 * - VBUS voltage in millivolts (2 bytes, MSB first, or 0x0000 if VBUS is not available).
 */
void setup() {
    int16_t state;

    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Power.begin();

    // disable Bluetooth
    btStop();

    M5.Display.setTextSize(1);
    M5.Display.setTextColor(YELLOW);
    M5.Display.setFont(&fonts::FreeMono9pt7b);
    M5.Display.setTextScroll(false);
    M5.Display.powerSave(false);

    Serial.begin(115200);
    Wire.begin();
    SPI.begin();
    if (!sht4.begin(&Wire, SHT40_I2C_ADDR_44, 2, 1, 400000U)) {
        debug(F("SHT4 not working"),  0);
    }
    if (bmp.begin(&Wire, BMP280_I2C_ADDR, 2, 1, 400000U)) {
        debug(F("BMP not working"),  0);
    }

    sht4.setPrecision(SHT4X_HIGH_PRECISION);
    sht4.setHeater(SHT4X_NO_HEATER);
    bmp.setSampling(BMP280::MODE_NORMAL,     /* Operating Mode.       */
                    BMP280::SAMPLING_X2,     /* Temp. oversampling    */
                    BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    BMP280::FILTER_X16,      /* Filtering.            */
                    BMP280::STANDBY_MS_500); /* Standby time.         */

    M5.Display.clear();
    M5.Display.setCursor(0, 0);

    print_wakeup_reason();

    state = lora_activate();

    if (state == RADIOLIB_LORAWAN_NEW_SESSION || state == RADIOLIB_LORAWAN_SESSION_RESTORED) {
        uint8_t uplink_buff[52];
        uint8_t uplink_buff_size = 0;

        // prepare payload
        bool    bat_ischarging = M5.Power.isCharging();
        int16_t bat_vol        = M5.Power.getBatteryVoltage(); // 0 .. 4095 voltage in millivolt
        int32_t bat_level      = M5.Power.getBatteryLevel();   // 0 .. 100  voltage percent
        int16_t vbus_vol       = M5.Power.getVBUSVoltage();    // 0 .. 4095 voltage in millivolt, -1 if not applicable
        float   temp           = 0.0f;
        float   humi           = 0.0f;
        float   pressure       = 0.0f;
        float   altitude       = 0.0f;

        if (sht4.update()) {
            temp     = sht4.cTemp;
            humi     = sht4.humidity;
        }
        if (bmp.update()) {
            pressure = bmp.pressure;
            altitude = bmp.altitude;
        }

        uplink_buff[uplink_buff_size++] = bat_ischarging ? 1 : 0;
        uplink_buff[uplink_buff_size++] = bat_vol >> 8;
        uplink_buff[uplink_buff_size++] = bat_vol & 0xFF;
        uplink_buff[uplink_buff_size++] = bat_level & 0xFF;
        uplink_buff[uplink_buff_size++] = vbus_vol == -1 ? 0x00 : vbus_vol >> 8;
        uplink_buff[uplink_buff_size++] = vbus_vol == -1 ? 0x00 : vbus_vol & 0xFF;
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&temp)[3];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&temp)[2];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&temp)[1];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&temp)[0];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&humi)[3];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&humi)[2];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&humi)[1];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&humi)[0];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&pressure)[3];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&pressure)[2];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&pressure)[1];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&pressure)[0];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&altitude)[3];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&altitude)[2];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&altitude)[1];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&altitude)[0];

        state = lora_send_receive(uplink_buff, uplink_buff_size, downlink_handler);
    }

    lora_save_session();

    deepsleep(TIME_TO_SLEEP);
}

// The ESP32 wakes from deep-sleep and starts from the very beginning.
// It then goes back to sleep, so loop() is never called and which is
// why it is empty.
void loop() {
    // never here
}
