#include "nvs_flash.h"
#include "nvs.h"
#include "string.h"
#include "esp_log.h"
#include "save_wifi.h"

char SSID_CONNECT[32] = "My_Default_SSID";
char PASSWORD_CONNECT[64] = "My_Default_Password";

// Hàm lưu trữ SSID và Password vào NVS
void store_wifi_credentials() {
    // Khởi tạo NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Mở NVS với tên không gian lưu trữ "storage"
    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        // Lưu SSID và Password vào NVS
        err = nvs_set_str(my_handle, "SSID", SSID_CONNECT);
        if (err != ESP_OK) {
            printf("Failed to write SSID\n");
        }
        
        err = nvs_set_str(my_handle, "Password", PASSWORD_CONNECT);
        if (err != ESP_OK) {
            printf("Failed to write Password\n");
        }

        // Lưu thay đổi vào NVS
        err = nvs_commit(my_handle);
        if (err != ESP_OK) {
            printf("Failed to commit NVS\n");
        }

        // Đóng NVS handle
        nvs_close(my_handle);
    }
}

// Hàm đọc SSID và Password từ NVS
void read_wifi_credentials() {
    // Khởi tạo NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Mở NVS với tên không gian lưu trữ "storage"
    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        // Đọc SSID từ NVS
        char ssid[32];
        size_t len = sizeof(ssid);
        err = nvs_get_str(my_handle, "SSID", ssid, &len);
        if (err == ESP_OK) {
            printf("SSID = %s\n", ssid);
            strcpy(SSID_CONNECT, ssid);
        } else {
            printf("Failed to read SSID\n");
        }

        // Đọc Password từ NVS
        char password[64];
        len = sizeof(password);
        err = nvs_get_str(my_handle, "Password", password, &len);
        if (err == ESP_OK) {
            printf("Password = %s\n", password);
            strcpy(PASSWORD_CONNECT, password);
        } else {
            printf("Failed to read Password\n");
        }

        // Đóng NVS handle
        nvs_close(my_handle);
    }
}
