#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedENV.h>

M5Canvas canvas(&M5.Display);

m5::unit::UnitUnified Units;

// #define USING_ENV3
#define USING_ENV4

#if defined(USING_ENV3)

m5::unit::UnitENV3 unitENV3;
auto& sht30   = unitENV3.sht30;
auto& qmp6988 = unitENV3.qmp6988;

#elif defined(USING_ENV4)

m5::unit::UnitENV4 unitENV4;
auto& sht40  = unitENV4.sht40;
auto& bmp280 = unitENV4.bmp280;

#endif

float calculate_altitude(const float pressure, const float seaLvhPa = 1013.25f)
{
    return 44330.f * (1.0f - pow((pressure / 100.f) / seaLvhPa, 0.1903f));
}

void setup()
{
    M5.begin();
    Serial.begin(115200);
    M5.Display.setFont(&fonts::lgfxJapanMinchoP_20);
    M5.Display.setTextSize(1);
    auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
    auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
    Serial.printf("getPin: SDA:%u SCL:%u", pin_num_sda, pin_num_scl);
    Serial.println();

    Wire.begin(pin_num_sda, pin_num_scl, 400000U);

#if defined(USING_ENV3)
    if (!Units.add(unitENV3, Wire) || !Units.begin()) {
        M5_LOGE("Failed to begin Unit ENV3");
        M5.Display.clear(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }

#elif defined(USING_ENV4)
    if (!Units.add(unitENV4, Wire) || !Units.begin()) {
        M5_LOGE("Failed to begin Unit ENV4");
        M5.Display.clear(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }

#endif
}

void loop()
{
    M5.update();
    Units.update();
#if defined(USING_ENV3)

    Units.wait_for_update();

    if (sht30.updated()) {
        M5.Display.setCursor(0, 0);
        M5.Display.fillRect(0, 0, 320, 80, TFT_BLACK);
        M5.Display.printf(
            "\n>SHT30Temp:%.4f\n"
            ">Humidity:%.4f",
            sht30.temperature(), sht30.humidity());
    }
    if (qmp6988.updated()) {
        M5.Display.setCursor(0, 80);
        M5.Display.fillRect(0, 80, 320, 80, TFT_BLACK);
        auto p = qmp6988.pressure();
        M5.Display.printf(
            "\n>QMP6988Temp:%.4f\n"
            ">Pressure:%.4f\n"
            ">Altitude:%.4f",
            qmp6988.temperature(), p * 0.01f /* To hPa */, calculate_altitude(p));
    }

#elif defined(USING_ENV4)
    if (sht40.updated()) {
        M5.Display.setCursor(0, 0);
        M5.Display.fillRect(0, 0, 320, 80, TFT_BLACK);
        M5.Display.printf(
            "\n>SHT40Temp: %.4f\n"
            ">Humidity :%.4f",
            sht40.temperature(), sht40.humidity());
    }
    if (bmp280.updated()) {
        M5.Display.setCursor(0, 80);
        M5.Display.fillRect(0, 80, 320, 80, TFT_BLACK);
        auto p = bmp280.pressure();
        M5.Display.printf(
            "\n>BMP280Temp: %.4f\n"
            ">Pressure: %.4f\n"
            ">Altitude: %.4f",
            bmp280.temperature(), p * 0.01f /* To hPa */, calculate_altitude(p));
    }
#endif
    delay(1000);
}