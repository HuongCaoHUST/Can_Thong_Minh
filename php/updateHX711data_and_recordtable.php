<?php
  require 'database.php';
  
  //---------------------------------
  if (!empty($_POST)) {
    //.............................
    $id = $_POST['id'];
    $weight = $_POST['weight'];
    $name = $_POST['name'];
    $status_read_sensor_hx711 = $_POST['status_read_sensor_hx711'];
    $led_01 = $_POST['led_01'];
    $led_02 = $_POST['led_02'];
    //........................................
    
    //................................
    date_default_timezone_set("Asia/Ho_Chi_Minh"); 
    $tm = date("H:i:s");
    $dt = date("Y-m-d");
    //........................................
    
    //.............................
    $pdo = Database::connect();
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
 
    $sql = "UPDATE 	esp8266_table_hx711_leds_update	 SET  weight = ?, name =?,status_read_sensor_hx711 = ?, time = ?, date = ? WHERE id = ?";
    $q = $pdo->prepare($sql);
    $q->execute(array($weight,$name,$status_read_sensor_hx711,$tm,$dt,$id));
    Database::disconnect();
    //..................................
    
    //..................................
    $id_key;
    $board = $_POST['id'];
    $found_empty = false;
    
    $pdo = Database::connect();
    
    //:::::::: 
    while ($found_empty == false) {
      $id_key = generate_string_id(10);
     
      $sql = 'SELECT * FROM esp8266_table_hx711_leds_record WHERE id="' . $id_key . '"';
      $q = $pdo->prepare($sql);
      $q->execute();
      
      if (!$data = $q->fetch()) {
        $found_empty = true;
      }
    }
    //::::::::
    
    //:::::::: 
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

		$sql = "INSERT INTO esp8266_table_hx711_leds_record (id,board,name,weight,status_read_sensor_hx711,LED_01,LED_02,time,date) values(?, ?, ?, ?, ?, ?, ?, ?, ?)";
		$q = $pdo->prepare($sql);
		$q->execute(array($id_key,$board,$name,$weight,$status_read_sensor_hx711,$led_01,$led_02,$tm,$dt));
    //::::::::
    
    Database::disconnect();
    //.............
  }
 
  function generate_string_id($strength = 16) {
    $permitted_chars = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
    $input_length = strlen($permitted_chars);
    $random_string = '';
    for($i = 0; $i < $strength; $i++) {
      $random_character = $permitted_chars[mt_rand(0, $input_length - 1)];
      $random_string .= $random_character;
    }
    return $random_string;
  }
  //---------------
?>