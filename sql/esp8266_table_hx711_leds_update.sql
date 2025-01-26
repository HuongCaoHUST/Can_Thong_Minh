//---------------------------------------- Final
CREATE TABLE `esp8266_table_hx711_leds_update` (
    `id` varchar(255) NOT NULL,
    `name` varchar(255) NOT NULL,
    `weight` float(10,2) NOT NULL,
    `status_read_sensor_hx711` varchar(255) NOT NULL,
    `LED_01` varchar(255) NOT NULL,
    `LED_02` varchar(255) NOT NULL,
    `time` time NOT NULL,
    `date` date NOT NULL,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;