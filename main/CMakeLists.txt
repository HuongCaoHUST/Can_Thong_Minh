idf_component_register(SRCS "app_main.c" "http_server_app.c" "save_wifi.c" "output_iot.c" "input_iot.c" "lcd.c"
                    INCLUDE_DIRS "."
                    REQUIRES http_server_app hd44780 pcf8574 hx711)