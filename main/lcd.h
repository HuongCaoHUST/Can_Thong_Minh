#ifndef LCD_H
#define LCD_H
#include "freertos/queue.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

extern i2c_dev_t pcf8574;
extern hd44780_t lcd;

typedef struct {
    char line1[17]; // Nội dung dòng 1 (tối đa 16 ký tự)
    char line2[17]; // Nội dung dòng 2 (tối đa 16 ký tự)
} lcd_content_t;
extern QueueHandle_t lcd_queue;
void lcd_init();
void lcd_manager_task(void *pvParameters);
void send_to_lcd(const char *line1, const char *line2);
void send_to_lcd_5s(const char *line1, const char *line2);
#endif