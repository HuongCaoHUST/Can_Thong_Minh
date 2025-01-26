#include "http_server_app.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include <esp_http_server.h>
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"

#if !CONFIG_IDF_TARGET_LINUX
#include <esp_wifi.h>
#include <esp_system.h>
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "cJSON.h"
#endif  // !CONFIG_IDF_TARGET_LINUX
httpd_handle_t server = NULL;

void start_webserver(void)
{
        printf("Hello");
//     httpd_config_t config = HTTPD_DEFAULT_CONFIG();
// #if CONFIG_IDF_TARGET_LINUX

//     config.server_port = 8001;
// #endif // !CONFIG_IDF_TARGET_LINUX
//     config.lru_purge_enable = true;

//     //wifi_scan();
//     // Start the httpd server
//     ESP_LOGI(TAG1, "Starting server on port: '%d'", config.server_port);
//     if (httpd_start(&server, &config) == ESP_OK) {
//         // Set URI handlers 
//         ESP_LOGI(TAG1, "Registering URI handlers");
//         httpd_register_uri_handler(server, &get_dht11);
//         httpd_register_uri_handler(server, &post_data);
//         httpd_register_uri_handler(server, &switch1_post_data);
//         httpd_register_uri_handler(server, &get_data_dht11);
//         httpd_register_uri_handler(server, &wifi_post_uri);
//         httpd_register_uri_handler(server, &wifi_login_uri);
//         httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, http_404_error_handler);
//     }
//     else{
//         ESP_LOGI(TAG1, "Error starting server!");
//     }
}

#if !CONFIG_IDF_TARGET_LINUX

void stop_webserver(void)
{
    // Stop the httpd server
    httpd_stop(server);
}
#endif // !CONFIG_IDF_TARGET_LINUX
