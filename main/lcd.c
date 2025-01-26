#include <inttypes.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <hd44780.h>
#include <pcf8574.h>
#include <string.h>
#include "freertos/queue.h"
#include "lcd.h"

i2c_dev_t pcf8574;
hd44780_t lcd;
static esp_err_t write_lcd_data(const hd44780_t *lcd, uint8_t data)
{
    return pcf8574_port_write(&pcf8574, data);
}

void lcd_init()
{
    // Cấu hình LCD chỉ một lần
    lcd = (hd44780_t) {
        .write_cb = write_lcd_data,
        .font = HD44780_FONT_5X8,
        .lines = 2,
        .pins = {
            .rs = 0,
            .e  = 2,
            .d4 = 4,
            .d5 = 5,
            .d6 = 6,
            .d7 = 7,
            .bl = 3
        }
    };

    // Khởi tạo PCF8574 (I2C)
    memset(&pcf8574, 0, sizeof(i2c_dev_t));
    ESP_ERROR_CHECK(pcf8574_init_desc(&pcf8574, CONFIG_EXAMPLE_I2C_ADDR, 0, CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL));

    // Khởi tạo LCD
    ESP_ERROR_CHECK(hd44780_init(&lcd));

    // Bật đèn nền LCD
    hd44780_switch_backlight(&lcd, true);
}

void lcd_manager_task(void *pvParameters)
{
    lcd_content_t current_content = {"", ""}; // Nội dung hiện tại trên LCD
    lcd_content_t new_content;

    while (true)
    {
        // Chờ nội dung mới từ hàng đợi
        if (xQueueReceive(lcd_queue, &new_content, portMAX_DELAY) == pdTRUE)
        {
            // Cập nhật dòng 1 nếu thay đổi
            if (strcmp(current_content.line1, new_content.line1) != 0)
            {
                // Tính toán vị trí bắt đầu để căn giữa
                int len1 = strlen(new_content.line1);
                int start_pos1 = (16 - len1) / 2; // Giả sử màn hình LCD 16x2

                hd44780_gotoxy(&lcd, 0, 0);
                hd44780_puts(&lcd, "                "); // Xóa dòng
                hd44780_gotoxy(&lcd, start_pos1, 0);
                hd44780_puts(&lcd, new_content.line1);

                strcpy(current_content.line1, new_content.line1);
            }

            // Cập nhật dòng 2 nếu thay đổi
            if (strcmp(current_content.line2, new_content.line2) != 0)
            {
                // Tính toán vị trí bắt đầu để căn giữa
                int len2 = strlen(new_content.line2);
                int start_pos2 = (16 - len2) / 2;

                hd44780_gotoxy(&lcd, 0, 1);
                hd44780_puts(&lcd, "                "); // Xóa dòng
                hd44780_gotoxy(&lcd, start_pos2, 1);
                hd44780_puts(&lcd, new_content.line2);

                strcpy(current_content.line2, new_content.line2);
            }
        }
    }
}


void send_to_lcd(const char *line1, const char *line2)
{
    lcd_content_t content;
    strncpy(content.line1, line1, 16);
    content.line1[16] = '\0'; // Đảm bảo kết thúc chuỗi
    strncpy(content.line2, line2, 16);
    content.line2[16] = '\0';

    // Gửi nội dung vào hàng đợi
    if (xQueueSend(lcd_queue, &content, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        ESP_LOGW("send_to_lcd", "LCD queue full");
    }
}

void send_to_lcd_5s(const char *line1, const char *line2)
{
    lcd_content_t content;
    strncpy(content.line1, line1, 16);
    content.line1[16] = '\0'; // Đảm bảo kết thúc chuỗi
    strncpy(content.line2, line2, 16);
    content.line2[16] = '\0';

    // Gửi nội dung vào hàng đợi
    if (xQueueSend(lcd_queue, &content, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        ESP_LOGW("send_to_lcd", "LCD queue full");
    }

    // Tạm dừng 2 giây để hiển thị nội dung
    vTaskDelay(pdMS_TO_TICKS(5000));
}
