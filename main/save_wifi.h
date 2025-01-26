#ifndef _SAVE_WIFI_H
#define _SAVE_WIFI_H
#include "esp_err.h"

extern char SSID_CONNECT[32];
extern char PASSWORD_CONNECT[64];

void store_wifi_credentials();
void read_wifi_credentials();
#endif