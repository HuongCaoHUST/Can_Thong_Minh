<?php
  include 'database.php';
  
  //---------------------------------------
  if (!empty($_POST)) {
   
    $id = $_POST['id'];
    
    $myObj = (object)array();
    
    
    $pdo = Database::connect();
   
    $sql = 'SELECT * FROM esp8266_table_hx711_leds_update WHERE id="' . $id . '"';
    foreach ($pdo->query($sql) as $row) {
      $date = date_create($row['date']);
      $dateFormat = date_format($date,"d-m-Y");
      
      $myObj->id = $row['id'];
      $myObj->name = $row['name'];
      $myObj->weight = $row['weight'];
      $myObj->status_read_sensor_hx711 = $row['status_read_sensor_hx711'];
      $myObj->LED_01 = $row['LED_01'];
      $myObj->LED_02 = $row['LED_02'];
      $myObj->ls_time = $row['time'];
      $myObj->ls_date = $dateFormat;
      
      $myJSON = json_encode($myObj);
      
      echo $myJSON;
    }
    Database::disconnect();
    //........................................ 
  }
  //---------------------------------------- 
?>