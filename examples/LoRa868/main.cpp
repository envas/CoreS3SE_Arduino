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

  For M5Stack Units see
  https://docs.m5stack.com/en/arduino/projects/unit/unit_env

*/

#include <Arduino.h>
#include <M5GFX.h>
#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedENV.h>
#include <RadioLib.h>

#include "main.h"

#include <BMP280.h>

#include "lora.h"
#include "utils.h"

extern RTC_DATA_ATTR uint16_t bootCount;

// display
M5Canvas canvas(&M5.Display);

// all units
m5::unit::UnitUnified Units;

// environmental sensor
m5::unit::UnitENV4 unitENV4;
auto&              sht40  = unitENV4.sht40;
auto&              bmp280 = unitENV4.bmp280;

void bmp280_sleep() {
    // bmp280.writeRegister8(0xF4, static_cast<uint8_t>(0xFC), true);
}

void bmp280_wakeup() {
    // bmp280.writeRegister8(0xF4, 0x3F, 1);
}

/**
 * Calculate altitude based on the pressure
 *
 * @param pressure  atmospherical pressure
 * @param seaLvhPa  pressure for the see level
 *
 * @return altitude in meters
 */
float calculate_altitude(const float pressure, const float seaLvhPa = 1013.25f) {
    return 44330.f * (1.0f - pow((pressure / 100.f) / seaLvhPa, 0.1903f));
}

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
        M5.Display.println(F("Boot up"));
    }

    M5.Display.print(F("Boot count: "));
    M5.Display.println(++bootCount); // increment before printing
}

/**
 * Puts the device into the lowest power deep-sleep mode
 *
 * @param seconds   sleep duration
 */
void deepsleep(uint32_t seconds) {
    M5.Display.println("Going to sleep ...");

    // give the user a chance to read the display
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    // put LoRa into the lowest power deep-sleep mode
    lora_sleep();

    // turn off the display before entering deep sleep (ESP.deepSleep does not do it)
    M5.Display.sleep();
    M5.Display.powerSave(true);

    // enter deep sleep  for `seconds`
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
 * - Sets up serial communication.
 * - Displays text settings on the screen.
 * - Prints the wakeup reason after a reset or wakeup from deep sleep.
 * - Activates the LoRaWAN session, determines if a new session is created
 *   or restored, and prepares the payload data to send via LoRaWAN.
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
 * - temperature (4 bytes as floating point)
 * - humidity (4 bytes as floating point)
 * - athmospheric pressure in hPa (4 bytes as floating point)
 * - altitude (4 bytes as floating point, calculated from pressure)
 *
 */
void setup() {
    int16_t state;

    Serial.begin(115200);

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

    print_wakeup_reason();

    auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
    auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);

    Wire.begin(pin_num_sda, pin_num_scl, 400000U);

    if (!Units.add(unitENV4, Wire) || !Units.begin()) {
        M5.Display.clear(TFT_RED);
        M5.Display.setCursor(0, 0);
        M5.Display.println("Failed to begin Unit ENV4");
        while (true) {
            m5::utility::delay(10000);
        }
    }

    bmp280.writeOversamplingPressure(m5::unit::bmp280::Oversampling::X8);
    bmp280.writeOversamplingTemperature(m5::unit::bmp280::Oversampling::X8);
    bmp280.writeStandbyTime(m5::unit::bmp280::Standby::Time4sec);
    bmp280.writePowerMode(m5::unit::bmp280::PowerMode::Normal);
    bmp280.writeUseCaseSetting(m5::unit::bmp280::UseCase::LowPower);

    M5.update();
    sht40.update(true);
    bmp280.update(true);

    while (!(sht40.updated() )) {
        sht40.update(true);
        delay(500);
    }

    while (!(bmp280.updated() )) {
        bmp280.update(true);
        delay(500);
    }

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
        float   pressure_hpa   = 0.0f;
        float   altitude       = 0.0f;
        float   ttemp          = 0.0f;



        temp         = sht40.temperature();
        humi         = sht40.humidity();
        pressure     = bmp280.pressure();
        ttemp        = bmp280.temperature();
        altitude     = calculate_altitude(pressure);
        pressure_hpa = pressure / 100.0f;

        M5.Display.fillRect(0, 0, 320, 60, TFT_BLACK);
        M5.Display.setCursor(0, 0);
        M5.Display.printf(">Temperature: %.1f\n", temp);
        M5.Display.printf(">Humidity:    %.1f\n", humi);
        M5.Display.printf(">Pressure:    %.1f\n", pressure_hpa);
        M5.Display.printf(">Altitude:    %.1f\n", altitude);
        M5.Display.printf(">BMP280 temp: %.1f\n", ttemp);
        M5.Display.println();

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
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&pressure_hpa)[3];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&pressure_hpa)[2];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&pressure_hpa)[1];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&pressure_hpa)[0];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&altitude)[3];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&altitude)[2];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&altitude)[1];
        uplink_buff[uplink_buff_size++] = ((uint8_t*)&altitude)[0];

        lora_send_receive(uplink_buff, uplink_buff_size, downlink_handler);
        lora_save_session();

        deepsleep(TIME_TO_SLEEP);
    } else {
        M5.Display.clear(TFT_RED);
        M5.Display.setCursor(0, 0);
        M5.Display.println("Failed to activate LoRaWAN session");
        while (true) {
            m5::utility::delay(10000);
        }
    }
}

// The ESP32 wakes from deep-sleep and starts from the very beginning.
// It then goes back to sleep, so loop() is never called and which is
// why it is empty.
void loop() {
    // never here
}
