#include <stdio.h>
#include <driver/gpio.h>
#include "output_iot.h"

// Hàm tạo GPIO đầu ra
void output_io_create(gpio_num_t gpio_num)
{
    // Cấu hình chân GPIO làm đầu ra
    gpio_set_direction(gpio_num, GPIO_MODE_OUTPUT);  
}

// Hàm thiết lập mức tín hiệu của GPIO
void output_io_set_level(gpio_num_t gpio_num, int level)
{  
    gpio_set_level(gpio_num, level);  // Cài đặt mức tín hiệu (0 hoặc 1)
}

// Hàm đảo trạng thái của GPIO (toggle)
void output_io_toggle(gpio_num_t gpio_num)
{  
    int old_level = gpio_get_level(gpio_num);  // Lấy trạng thái hiện tại của GPIO
    gpio_set_level(gpio_num, 1 - old_level);  // Đảo trạng thái (0 -> 1, 1 -> 0)
}
