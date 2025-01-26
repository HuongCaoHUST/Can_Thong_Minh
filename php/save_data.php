<?php
require 'database.php';

function generate_string_id($strength = 16) {
    $permitted_chars = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
    $input_length = strlen($permitted_chars);
    $random_string = '';
    for ($i = 0; $i < $strength; $i++) {
        $random_character = $permitted_chars[mt_rand(0, $input_length - 1)];
        $random_string .= $random_character;
    }
    return $random_string;
}

if ($_SERVER['REQUEST_METHOD'] == 'POST' && isset($_POST['save'])) {
    try {
        $pdo = Database::connect();
        $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

        //
        $sql_select = "SELECT id, name, weight, status_read_sensor_hx711, time, date FROM esp8266_table_hx711_leds_update";
        $stmt = $pdo->query($sql_select);
        $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);

        // 
        $sql_insert = "INSERT INTO esp8266_record_hx711_luu (id, board, name, weight, status_read_sensor_hx711, time, date) 
                       VALUES (?, ?, ?, ?, ?, ?, ?)";
        $stmt_insert = $pdo->prepare($sql_insert);
//
        foreach ($rows as $row) {
            $random_id = generate_string_id(10);
            $stmt_insert->execute([
                $random_id,
                $row['id'], 
                $row['name'],
                $row['weight'],
                $row['status_read_sensor_hx711'],
                $row['time'],
                $row['date']
            ]);
        }

        echo "Dữ liệu đã được lưu thành công!";
        Database::disconnect();
    } catch (Exception $e) {
        echo "Lỗi khi lưu dữ liệu: " . $e->getMessage();
    }
} else {
    echo "Yêu cầu không hợp lệ!";
}
