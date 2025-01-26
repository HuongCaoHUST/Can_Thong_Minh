<?php
  require 'database.php';
  
  //-------------------------------
  if (!empty($_POST)) {
    //............................
    $id = $_POST['id'];
    $lednum = $_POST['lednum'];
    $ledstate = $_POST['ledstate'];
    //........................................ 
    
    //........................................ 
    $pdo = Database::connect();
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    // 
    
    $sql = "UPDATE esp8266_table_hx711_leds_update SET " . $lednum . " = ? WHERE id = ?";
    $q = $pdo->prepare($sql);
    $q->execute(array($ledstate,$id));
    Database::disconnect();
    //........................................ 
  }
  //---------------------------------------- 
?>