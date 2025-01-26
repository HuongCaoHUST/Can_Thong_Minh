CREATE TABLE `esp8266_record_hx711_luu` (
    `id` varchar(255) NOT NULL,
    `board` varchar(255) NOT NULL,
    `name` float(10,2) NOT NULL,
    `weight` int(3) NOT NULL,
    `status_read_sensor_hx711` varchar(255) NOT NULL,
    `LED_01` varchar(255) NOT NULL,
    `LED_02` varchar(255) NOT NULL,
    `time` time NOT NULL,
    `date` date NOT NULL,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;