#ifndef INPUT_IO_H
#define INPUT_IO_H
#include "esp_err.h"
#include "driver/gpio.h"

typedef void (*input_callback_t) (int);

typedef enum {
    LO_TO_HI = GPIO_INTR_POSEDGE,   // Cạnh lên
    HI_TO_LO = GPIO_INTR_NEGEDGE,   // Cạnh xuống
    ANY_EDLE = GPIO_INTR_ANYEDGE    // Cả hai cạnh
} interrupt_type_edle_t;

void input_io_create(gpio_num_t gpio_num, interrupt_type_edle_t type);
void input_io_get_level(gpio_num_t gpio_num);
void input_set_callback(void * cb);

#endif
