//
// Created by Robert Carnecky on 05.09.2025.
//

#ifndef CORES3SE_ARDUINO_LORA_H
#define CORES3SE_ARDUINO_LORA_H


int16_t lora_activate();
void    lora_save_session();
int16_t lora_send_receive(uint8_t* data_up, size_t data_up_len, void (*callback)(uint8_t*, size_t)) ;

#endif //CORES3SE_ARDUINO_LORA_H
