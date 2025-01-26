#ifndef _HTTP_SERVER_APP_H
#define _HTTP_SERVER_APP_H
#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define WIFI_SSID_MAX_LEN 32
#define WIFI_PASS_MAX_LEN 64


extern char received_ssid[WIFI_SSID_MAX_LEN];
extern char received_password[WIFI_PASS_MAX_LEN];
extern EventGroupHandle_t received_data_wifi;
#define WIFI_CREDENTIALS_RECEIVED_BIT BIT0

void start_webserver(void);
void stop_webserver(void);
void wifi_scan(void);
void register_wifi_uri_handlers(httpd_handle_t server);
//void wifi_init_sta(const char *ssid, const char *password);

#endif