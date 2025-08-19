#include <Arduino.h>
#include <M5GFX.h>
#include <M5Unified.h>

void setup()
{
    auto cfg = M5.config();
    // if using ext power input(Grove Port or DC input power supply) needs to be set to false.
    // cfg.output_power = false;
    M5.begin(cfg);
    M5.Display.setTextSize(2);

    M5.Power.setChargeCurrent(200);
}

void loop()
{
    M5.Display.clear();

    bool bat_ischarging = M5.Power.isCharging();
    M5.Display.setCursor(10, 30);
    M5.Display.printf("Bat Charging: %s", bat_ischarging == 1 ? "Yes" : "No");

    int bat_vol = M5.Power.getBatteryVoltage();
    M5.Display.setCursor(10, 50);
    M5.Display.printf("Bat Voltage: %dmv", bat_vol);

    int bat_level = M5.Power.getBatteryLevel();
    M5.Display.setCursor(10, 70);
    M5.Display.printf("Bat Level: %d", bat_level);

    int vbus_vol = M5.Power.getVBUSVoltage();
    M5.Display.setCursor(10, 90);
    M5.Display.printf("VBus Voltage: %dmv", vbus_vol);
    delay(1000);
}