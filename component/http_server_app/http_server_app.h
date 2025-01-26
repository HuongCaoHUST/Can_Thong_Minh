#ifndef _HTTP_SERVER_APP_H
#define _HTTP_SERVER_APP_H
#include "esp_http_server.h"

void start_webserver(void);
void stop_webserver(void);
void wifi_scan(void);
void register_wifi_uri_handlers(httpd_handle_t server);
//void wifi_init_sta(const char *ssid, const char *password);

#endif