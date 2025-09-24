## Priklad pro M5Stamp PLC

* SKU:SKU:K141
* [Documentace](https://docs.m5stack.com/en/module/Module-LoRa868_V1.1)
* [Knihovna a priklady](https://github.com/m5stack/M5StamPLC)

StamPLC je programovatelný logický řadič IoT určený pro průmyslovou automatizaci a vzdálený monitoring. 
Produkt je založen na modulu StampS3A, který nejenže nabízí výkonné zpracovatelské schopnosti, ale také 
zajišťuje efektivní bezdrátové připojení. Z hlediska řízení nabízí StamPLC 8 opticky izolovaných digitálních vstupů a 
4 reléové výstupy (podporující jak střídavé, tak stejnosměrné zátěže), spolu s portem GPIO.EXT a 2 rozhraními HY2.0-4P, 
což umožňuje jednoduchou a spolehlivou integraci různých senzorů a akčních členů. Díky integrovaným rozhraním 
PWR-CAN a PWR-485 lze zařízení hladce integrovat do průmyslových sběrnicových sítí, což umožňuje vzdálený přenos 
dat a centralizované řízení. Pro interakci mezi člověkem a strojem je produkt vybaven 1,14palcovým barevným displejem, 
tlačítkem RESET/BOOT, 3 uživatelskými tlačítky a bzučákem, které usnadňují konfiguraci parametrů v reálném čase a 
monitorování stavu a mohou uživatele upozornit v případě anomálií. Aby odolal drsným průmyslovým podmínkám, 
podporuje StamPLC široký rozsah vstupního napětí (6–36 V DC) a je navržen pro montáž na DIN lištu, která zajišťuje 
bezpečnou instalaci. Vestavěný slot pro microSD kartu dále usnadňuje ukládání dat a aktualizace firmwaru. 
Kromě toho je do systému monitorování prostředí integrován teplotní senzor LM75 a senzor napětí/proudu INA226 
pro zpětnou vazbu v reálném čase o provozu zařízení, zatímco modul RTC (RX8130CE) zajišťuje přesnou synchronizaci 
času a záznam protokolů. Tovární firmware automaticky nahrává data na cloudovou platformu M5 EZData, generuje 
monitorovací stránky a nabízí uživatelům pohodlný vzdálený přístup a ovládání přes cloud. Tento produkt je vhodný 
pro průmyslovou automatizaci, vzdálený monitoring, inteligentní výrobu a další aplikace.

### M5StamPLC a Serial

PlatformIO vůbec nepoužívá jádro Arduino specifické pro M5Stack. Používá standardní jádro Espressif
(https://github.com/espressif/arduino-esp32/). V případě M5StampS3 je při pohledu na schémata vidět,
že na desce není žádný čip adaptéru USB-UART, ESP32S3 je připojen přímo k zástrčce USB-C. Proto by Serial
měl používat nativní funkce USB-CDC čipu ESP32S3. Arduino IDE (jádro Espressif) i PlatformIO však tuto funkci
mají vypnutou. Proto je třeba kompilovat kód pro M5StampS3 s flagem `ARDUINO_USB_CDC_ON_BOOT=1`:

```
build_flags =
    -DARDUINO_USB_CDC_ON_BOOT=1
```

### Modbus

Modbus example for StamPLC poziva eModbus library

https://emodbus.github.io/