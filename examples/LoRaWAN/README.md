
## Konfigurace

Parametry modulu, ktere mohou byt dulezite pri konfigurovani

```
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
```

Parametry NSS, IRQ a RST musi byt nastaveny na DIP prepinacich na desce. Hodnoty `NSS=0, RST=7, IRQ=10` 
pro mne fungovaly OK.

**Dulezite**: radio modul objekt musi byt definovan jako SX1276, objekt SX1278 nefunguje.

```
SX1276 radio = new Module(CONFIG_LORA_NSS, CONFIG_LORA_IRQ, CONFIG_LORA_RST, CONFIG_LORA_BUSY);
```