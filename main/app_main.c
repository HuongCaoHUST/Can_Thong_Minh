#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <hx711.h>

#include "lwip/err.h"
#include "lwip/sys.h"
#include "http_server_app.h"
#include "save_wifi.h"
#include "output_iot.h"
#include "driver/gpio.h"
#include <hd44780.h>
#include <pcf8574.h>
#include "input_iot.h"
#include <inttypes.h>
#include <lcd.h>
#include "protocol_examples_common.h"
#include "esp_tls.h"
#include "esp_netif.h"
#include "esp_http_client.h"

#define LED_PIN 2
//#define EXAMPLE_ESP_WIFI_SSID   "Phòng 3.2"
//#define EXAMPLE_ESP_WIFI_PASS   "0917973730"
#define WIFI_SSID_AP      "SMART_SCALE_CONFIG"
#define WIFI_PASS_AP      "smartscale"
#define EXAMPLE_MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN

#include "freertos/semphr.h"
QueueHandle_t lcd_queue;
SemaphoreHandle_t lcd_mutex;
EventGroupHandle_t wifi_status;
EventGroupHandle_t received_data_wifi;
#define WIFI_CREDENTIALS_RECEIVED_BIT BIT0

#define WIFI_SSID_MAX_LEN 32
#define WIFI_PASS_MAX_LEN 64

char received_ssid[WIFI_SSID_MAX_LEN] = {0};
char received_password[WIFI_PASS_MAX_LEN] = {0};
char IP_ADDRESS[20];

/*Che do SOFT_AP*/
static const char *TAG = "wifi softAP";
#define TAG4 "LED"

static int WIFI_STATUS = 0;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap()
{
    tcpip_adapter_init();
    //ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID_AP,
            .ssid_len = strlen(WIFI_SSID_AP),
            .password = WIFI_PASS_AP,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(WIFI_PASS_AP) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s",
             WIFI_SSID_AP, WIFI_PASS_AP);
}

/*Che do STATION*/
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG1 = "wifi station";

static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG1,"connect to the AP fail");
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG1, "got ip:%s",
                 ip4addr_ntoa(&event->ip_info.ip));
        snprintf(IP_ADDRESS, sizeof(IP_ADDRESS), "%s", ip4addr_ntoa(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    
    wifi_config_t wifi_config = { 0 };
    strcpy((char *)wifi_config.sta.ssid, SSID_CONNECT);
	strcpy((char *)wifi_config.sta.password, PASSWORD_CONNECT);

    if (strlen((char *)wifi_config.sta.password)) {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG1, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG1, "connected to ap SSID:%s password:%s",
                 SSID_CONNECT, PASSWORD_CONNECT);
        WIFI_STATUS = 1;
        send_to_lcd("CONNECTED TO", (char *)SSID_CONNECT);
        vTaskDelay(pdMS_TO_TICKS(3000));
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG1, "Failed to connect to SSID:%s, password:%s",
                 SSID_CONNECT, PASSWORD_CONNECT);
        send_to_lcd("CONNEC FAIL", " ");
    } else {
        ESP_LOGE(TAG1, "UNEXPECTED EVENT");
    }

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    vEventGroupDelete(s_wifi_event_group);
}

void check_receive_data_wifi (void *pvParameters){
    while (1) {
    // Chờ bit sự kiện được đặt
    EventBits_t bits = xEventGroupWaitBits(
        received_data_wifi,
        WIFI_CREDENTIALS_RECEIVED_BIT,
        pdTRUE,   // Xóa bit sau khi nhận
        pdFALSE,  // Không cần chờ tất cả các bit
        portMAX_DELAY // Chờ vô thời hạn
    );
        if (bits & WIFI_CREDENTIALS_RECEIVED_BIT) {
            ESP_LOGI("main", "Received Data Wifi SSID: %s", received_ssid);
            send_to_lcd("CONNECTING TO:", (char *)received_ssid);
            strcpy(SSID_CONNECT, (char *)received_ssid);
            strcpy(PASSWORD_CONNECT, (char *)received_password);
            store_wifi_credentials();
            read_wifi_credentials();

            ESP_LOGI(TAG, "Switching to STA mode...");
            esp_restart();

            if(WIFI_STATUS == 1)
            {
                send_to_lcd("CONNECTED TO:", (char *)received_ssid);
            }
            else {
                send_to_lcd("CONNECT FAIL", " ");
                vTaskDelay(pdMS_TO_TICKS(2000));
                send_to_lcd("CONFIG WIFI", "192.168.4.1");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048
static const char *TAG_HTTP = "HTTP_CLIENT";

// Khai báo hàm
static void send_weight_to_server(float weight);
static void http_rest_with_url(void);


static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG_HTTP, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG_HTTP, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            break;
        default:
            break;
    }
    return ESP_OK;
}

static void send_weight_to_server(float weight) {
    char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    esp_http_client_config_t config = {
        .url = "http://192.168.43.166/finalend/updateHX711data_and_recordtable.php", // Thay URL bằng server của bạn
        .event_handler = _http_event_handler,
        .user_data = local_response_buffer,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Định dạng dữ liệu POST theo x-www-form-urlencoded
    char post_data[200];
    snprintf(post_data, sizeof(post_data), 
             "id=esp8266_01&weight=%.2f&name=Dao Duy Anh&status_read_sensor_hx711=SUCCESS&led_01=ON&led_02=ON", 
             weight);

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG_HTTP, "HTTP POST Status = %d, content_length = %d",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG_HTTP, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}


static const char *TAG6 = "hx711-example";
int32_t data;
float cannang = 0;
char cannang_str[20];
int max_weight = 160;
int max_ticks = 16;
int ticks = 0;
char weight_result[17];
void test(void *pvParameters)
{
    hx711_t dev = {
        .dout = 12,
        .pd_sck = 15,
        .gain = HX711_GAIN_A_64
    };

    // initialize device
    ESP_ERROR_CHECK(hx711_init(&dev));

    // read from device
    while (1)
    {
        esp_err_t r = hx711_wait(&dev, 500);
        if (r != ESP_OK)
        {
            ESP_LOGE(TAG6, "Device not found: %d (%s)\n", r, esp_err_to_name(r));
            continue;
        }
        r = hx711_read_average(&dev, CONFIG_EXAMPLE_AVG_TIMES, &data);
        if (r != ESP_OK)
        {
            ESP_LOGE(TAG6, "Could not read data: %d (%s)\n", r, esp_err_to_name(r));
            continue;
        }
    // Chuyển đổi raw data sang trọng lượng
    cannang = (data*0.000069016-8.093593849)-0.1;
    if(cannang <0)
    {
        cannang = 0;
    }
    ticks = (int)((cannang / max_weight) * max_ticks);
    char gauge[20];
    for (int i = 0; i < max_ticks; i++) {
        if (i < ticks) {
            gauge[i] = 0xFF;  // Vạch đầy
        } else {
            gauge[i] = ' ';  // Vạch trống
        }
    }
    gauge[max_ticks] = '\0';

    //ESP_LOGI(TAG, "Raw data: %" PRIi32 ", Weight: %.2f g", data, weight);
    snprintf(cannang_str, sizeof(cannang_str), "%.1f kg", cannang);
    printf("Raw data: %" PRIi32 ", Weight: %3.1f kg\n", data, cannang);
    
    strcpy(weight_result, "WEIGHT: ");
    strcat(weight_result, cannang_str);

    send_to_lcd(weight_result, gauge);

    if(WIFI_STATUS)
    {
        send_weight_to_server(cannang);
    }

    //send_to_lcd("WEIGHT",  cannang_str );
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

#define ESP_INTR_FLAG_DEFAULT 0
// Định nghĩa GPIO
#define BUTTON_GPIO GPIO_NUM_0
#define BUTTON_GPIO_2 GPIO_NUM_14

static QueueHandle_t gpio_evt_queue = NULL;

// ISR handler cho nút nhấn
static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

// Task xử lý sự kiện ngắt từ nút nhấn
static void gpio_task_example(void* arg) {
    uint32_t io_num;
    int led_state = 0;
    for (;;) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            if (io_num == BUTTON_GPIO) {
                esp_restart();
            } else if (io_num == BUTTON_GPIO_2) {
                xTaskCreate(test, "test", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL);
            }
        }
    }
}

void app_main()
{   
    // Cấu hình GPIO cho LED
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&io_conf);

    // Cấu hình GPIO cho nút nhấn với ngắt cạnh xuống
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << BUTTON_GPIO);
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);


    // Cấu hình GPIO cho nút nhấn BUTTON_GPIO_2 (GPIO 14)
    io_conf.pin_bit_mask = (1ULL << BUTTON_GPIO_2);
    gpio_config(&io_conf);

    // Tạo hàng đợi xử lý sự kiện ngắt
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    // Cài đặt dịch vụ ngắt và thêm handler cho nút nhấn
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(BUTTON_GPIO, gpio_isr_handler, (void*)BUTTON_GPIO);
    gpio_isr_handler_add(BUTTON_GPIO_2, gpio_isr_handler, (void*)BUTTON_GPIO_2);

    lcd_queue = xQueueCreate(5, sizeof(lcd_content_t));
    xTaskCreate(lcd_manager_task,"LCD Manager Task",configMINIMAL_STACK_SIZE * 5,NULL, 5 ,NULL);
    lcd_mutex = xSemaphoreCreateMutex();
    // Khởi tạo LCD
    ESP_ERROR_CHECK(i2cdev_init());  // Khởi tạo I2C
    lcd_init();  // Khởi tạo LCD chỉ một lần

    send_to_lcd("HELLO", "SMART SCALE");
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_ERROR_CHECK(nvs_flash_init());

    read_wifi_credentials();
    // strcpy(SSID_CONNECT, "Phòng 3.2");
    // strcpy(PASSWORD_CONNECT, "09179737303");
    ESP_LOGI(TAG1, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    start_webserver();
    printf("Trang thai: %d \n", WIFI_STATUS);

    //send_to_lcd("SMART SCALE IP", (char*)IP_ADDRESS );
    if (WIFI_STATUS)
    {
        ESP_LOGI(TAG1, "ESP_WIFI_MODE_STA");
        //wifi_init_sta();
        send_to_lcd("SMART SCALE IP", (char*)IP_ADDRESS );
    }
    else
    {   
        send_to_lcd("CONFIG WIFI", "192.168.4.1");
        ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
        ESP_ERROR_CHECK(esp_wifi_stop());
        esp_wifi_deinit();
        ESP_LOGI(TAG, "Switching to AP mode...");
        wifi_init_softap();
        start_webserver();
        //xTaskCreate(lcd_softAP, "lcd_softAP", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL);
    }
    printf("Trang thai: %d \n", WIFI_STATUS);

    received_data_wifi = xEventGroupCreate();
    xTaskCreate(check_receive_data_wifi, "check_receive_data_wifi", configMINIMAL_STACK_SIZE * 5, NULL, 6 , NULL);

    ESP_LOGI("TAG", "Weight: %.2f", cannang);
    printf("Scale: %f\n", cannang);
    vTaskDelay(pdMS_TO_TICKS(3000));
}