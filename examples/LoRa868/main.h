#ifndef _RADIOLIB_EX_LORAWAN_CONFIG_H
#define _RADIOLIB_EX_LORAWAN_CONFIG_H

#define TIME_TO_SLEEP  15 * 60         // sleep time in seconds (15 minutes)

// first you have to set your radio model and pin configuration
//
#define CONFIG_LORA_NSS  GPIO_NUM_0
#define CONFIG_LORA_BUSY GPIO_NUM_2
#define CONFIG_LORA_RST  GPIO_NUM_7
#define CONFIG_LORA_IRQ  GPIO_NUM_10

// joinEUI - previous versions of LoRaWAN called this AppEUI
// for development purposes you can use all zeros - see wiki for details
#define RADIOLIB_LORAWAN_JOIN_EUI  0x0000000000000000

// the Device EUI & two keys can be generated on the TTN console
#ifndef RADIOLIB_LORAWAN_DEV_EUI   // Replace with your Device EUI
#define RADIOLIB_LORAWAN_DEV_EUI   0x70B3D57ED0072797
#endif
#ifndef RADIOLIB_LORAWAN_APP_KEY   // Replace it with your App Key
#define RADIOLIB_LORAWAN_APP_KEY   0x50, 0x0E, 0xD5, 0x4E, 0xE3, 0x44, 0x1F, 0x18, 0x25, 0xAC, 0x20, 0xAE, 0xAF, 0xAE, 0xB0, 0x70
#endif
#ifndef RADIOLIB_LORAWAN_NWK_KEY   // Put your Nwk Key here
#define RADIOLIB_LORAWAN_NWK_KEY   0x50, 0x0E, 0xD5, 0x4E, 0xE3, 0x44, 0x1F, 0x18, 0x25, 0xAC, 0x20, 0xAE, 0xAF, 0xAE, 0xB0, 0x70
#endif

void deepsleep(uint32_t seconds);
void print_wakeup_reason();

#endif
