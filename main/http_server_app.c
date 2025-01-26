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
#include "save_wifi.h"

#endif  // !CONFIG_IDF_TARGET_LINUX
httpd_handle_t server = NULL;

static const char *TAG2 = "mainapp";
static const char *TAG3 = "Data_receive";

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

/* An HTTP GET handler */
static esp_err_t hello_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char*) index_html_start, index_html_end -index_html_start);
    return ESP_OK;
}

static const httpd_uri_t get_dht11 = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = hello_get_handler,
    .user_ctx  = NULL
};

static esp_err_t hello_handler(httpd_req_t *req)
{
    const char* resp_str = (const char*) "Xin chao nha";
    httpd_resp_send(req, resp_str, 13);  //Gửi dữ liệu ở đây
    return ESP_OK;
}

static const httpd_uri_t get_hello = {
    .uri       = "/dht11",
    .method    = HTTP_GET,
    .handler   = hello_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};

static char wifi_scan_results_json[2048];

void wifi_scan(void)
{
    // Chuyển Wi-Fi sang chế độ APSTA để cho phép quét
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

    // Cấu hình quét
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = false,  // Đặt thành false để ẩn các mạng SSID ẩn
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
    };

    // Bắt đầu quét
    esp_err_t err = esp_wifi_scan_start(&scan_config, true);
    if (err != ESP_OK) {
        ESP_LOGE(TAG2, "Wi-Fi scan failed: %s", esp_err_to_name(err));
        return;
    }

    // Lấy kết quả
    uint16_t ap_count = 0;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_LOGI(TAG2, "Found %d access points:", ap_count);

    wifi_ap_record_t ap_info[ap_count];
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_info));

    // Tạo JSON chứa kết quả quét Wi-Fi
    snprintf(wifi_scan_results_json, sizeof(wifi_scan_results_json),
        "[");
    for (int i = 0; i < ap_count; i++) {
        const char *ssid = strlen((char *)ap_info[i].ssid) > 0 ? (char *)ap_info[i].ssid : "<Hidden SSID>";
        snprintf(wifi_scan_results_json + strlen(wifi_scan_results_json), sizeof(wifi_scan_results_json) - strlen(wifi_scan_results_json),
            "{\"ssid\": \"%s\", \"bssid\": \"" MACSTR "\", \"rssi\": %d}",
            ssid, MAC2STR(ap_info[i].bssid), ap_info[i].rssi);
        if (i < ap_count - 1) {
            strcat(wifi_scan_results_json, ",");
        }
    }
    strcat(wifi_scan_results_json, "]");
}


// Handler cho POST request
esp_err_t wifi_post_handler(httpd_req_t *req) {
    // Quét Wi-Fi
    wifi_scan();

    // Set response type to JSON
    httpd_resp_set_type(req, "application/json");

    // Gửi lại dữ liệu quét Wi-Fi dưới dạng JSON
    if (wifi_scan_results_json[0] == '\0') {
        // Nếu không có dữ liệu, trả về lỗi hoặc thông báo trống
        httpd_resp_send(req, "{\"error\": \"No Wi-Fi networks found\"}", strlen("{\"error\": \"No Wi-Fi networks found\"}"));

    } else {
        httpd_resp_send(req, wifi_scan_results_json, strlen(wifi_scan_results_json));
    }
    return ESP_OK;
}

httpd_uri_t wifi_post_uri = {
            .uri = "/wifiscan",
            .method = HTTP_POST,
            .handler = wifi_post_handler,
            .user_ctx = NULL
};

// #define WIFI_SSID_MAX_LEN 32
// #define WIFI_PASS_MAX_LEN 64

// static char received_ssid[WIFI_SSID_MAX_LEN];
// static char received_password[WIFI_PASS_MAX_LEN];

esp_err_t wifi_login_handler(httpd_req_t *req) {
    // Đọc dữ liệu JSON gửi từ trang web
    char content[100];
    int ret = httpd_req_recv(req, content, req->content_len);
    if (ret < 0) {
        ESP_LOGE(TAG3, "Failed to read post data");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // Giải mã dữ liệu JSON
    cJSON *json = cJSON_Parse(content);
    if (json == NULL) {
        ESP_LOGE(TAG3, "Invalid JSON received");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // Lấy SSID và mật khẩu từ dữ liệu JSON
    const char *ssid = cJSON_GetObjectItem(json, "ssid")->valuestring;
    const char *password = cJSON_GetObjectItem(json, "password")->valuestring;

    // In SSID và mật khẩu ra console
    printf("Received SSID: %s\n", ssid);
    printf("Received Password: %s\n", password);

    if (ssid && password) {
        // Lưu SSID và mật khẩu vào các biến toàn cục
        strncpy(received_ssid, ssid, WIFI_SSID_MAX_LEN);
        strncpy(received_password, password, WIFI_PASS_MAX_LEN);
        xEventGroupSetBits(received_data_wifi, WIFI_CREDENTIALS_RECEIVED_BIT);
    }

    ESP_LOGI("wifi_login", "Received SSID: %s, Password: %s", received_ssid, received_password);
    // Gửi phản hồi thành công
    httpd_resp_send(req, "Wi-Fi connect request received", strlen("Wi-Fi connect request received"));
    // Giải phóng bộ nhớ JSON
    cJSON_Delete(json);

    return ESP_OK;
}


httpd_uri_t wifi_login_uri = {
    .uri = "/login",
    .method = HTTP_POST,
    .handler = wifi_login_handler,
    .user_ctx = NULL
};


void start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    ESP_LOGI(TAG2, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers 
        ESP_LOGI(TAG2, "Registering URI handlers");
        httpd_register_uri_handler(server, &get_dht11);
        httpd_register_uri_handler(server, &get_hello);
        httpd_register_uri_handler(server, &wifi_post_uri);
        httpd_register_uri_handler(server, &wifi_login_uri);
    }
}

#if !CONFIG_IDF_TARGET_LINUX

void stop_webserver(void)
{
    // Stop the httpd server
    httpd_stop(server);
}
#endif // !CONFIG_IDF_TARGET_LINUX
