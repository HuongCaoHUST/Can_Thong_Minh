<!DOCTYPE HTML>
<html>
  <head>
    <title>ESP8266 WITH MYSQL DATABASE</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
    <link rel="icon" href="data:,">
    <style>
      html {font-family: Arial; display: inline-block; text-align: center;}
      p {font-size: 1.2rem;}
      h4 {font-size: 0.8rem;}
      body {margin: 0;}
      .topnav {overflow: hidden; background-color: #0c6980; color: white; font-size: 1.2rem;}
      .content {padding: 5px; }
      .card {background-color: white; box-shadow: 0px 0px 10px 1px rgba(140,140,140,.5); border: 1px solid #0c6980; border-radius: 15px;}
      .card.header {background-color: #0c6980; color: white; border-bottom-right-radius: 0px; border-bottom-left-radius: 0px; border-top-right-radius: 12px; border-top-left-radius: 12px;}
      .cards {max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));}
      .reading {font-size: 1.3rem;}
      .packet {color: #bebebe;}
      .temperatureColor {color: #fd7e14;}
      .humidityColor {color: #1b78e2;}
      .statusreadColor {color: #702963; font-size:12px;}
      .LEDColor {color: #183153;}
    </style>
  </head>
  
  <body>
    <div class="topnav">
      <h3>ESP8266 WITH MYSQL DATABASE</h3>
    </div>
    
    <br>
    
    <!-- __ DISPLAYS MONITORIN ____________________________________________________________________________________________ -->
    <div class="content">
      <div class="cards">
        
        <!-- == MONITORING ======================================================================================== -->
        <div class="card">
          <div class="card header">
            <h3 style="font-size: 1rem;">MONITORING</h3>
          </div>
          
          <!-- Displays the weight values received from ESP8266. *** -->
          <h4 class="WeightColor"> WEIGHT </h4>
          <p class="WeightColor"><span class="reading"><span id="ESP8266_01_Weight"></span> kg</span></p>
          <!-- *********************************************************************** -->
          
          <p class="statusreadColor"><span>Status Read Sensor HX711 : </span><span id="ESP8266_01_Status_Read_hx711"></span></p>
        </div>
        <!-- ======================================================================================================= -->
      </div>
    </div>
    
    <br>
    
    <div class="content">
      <div class="cards">
        <div class="card header" style="border-radius: 15px;">
            <h3 style="font-size: 0.7rem;">LAST TIME RECEIVED DATA FROM 8266 [ <span id="ESP8266_01_LTRD"></span> ]</h3>
            <button onclick="window.open('luudulieu.php', '_blank');">Open Record Table</button>
            <button class="button" onclick="saveData()">Lưu Dữ Liệu</button>
          
            <h3 style="font-size: 0.7rem;"></h3>
        </div>
      </div>
    </div>
    <!-- ___________________________________________________________________________________________________________________________________ -->
    
    
    <script>
      //------------------------------------------------------------
      document.getElementById("ESP8266_01_Weight").innerHTML = "NN"; 
      document.getElementById("ESP8266_01_Status_Read_hx711").innerHTML = "NN";
      document.getElementById("ESP8266_01_LTRD").innerHTML = "NN";
      //------------------------------------------------------------
      
      Get_Data("8266_01");
      
      setInterval(myTimer, 5000);
      
      //------------------------------------------------------------
      function myTimer() {
        Get_Data("esp8266_01");
      }
      //------------------------------------------------------------
      
      //------------------------------------------------------------
      function Get_Data(id) {
        if (window.XMLHttpRequest) {
          xmlhttp = new XMLHttpRequest();
        } else {
          xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
        }
        xmlhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            const myObj = JSON.parse(this.responseText);
            if (myObj.id == "esp8266_01") {
              document.getElementById("ESP8266_01_Weight").innerHTML = myObj.weight;
              
              document.getElementById("ESP8266_01_Status_Read_hx711").innerHTML = myObj.status_read_sensor_hx711;
              document.getElementById("ESP8266_01_LTRD").innerHTML = "Time : " + myObj.ls_time + " | Date : " + myObj.ls_date + " (dd-mm-yyyy)";
            }
          }
        };
        xmlhttp.open("POST","getdata.php",true);
        xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
        xmlhttp.send("id="+id);
      }
      function saveData() {
  if (confirm("Bạn có chắc chắn muốn lưu dữ liệu không?")) {
    const xhr = new XMLHttpRequest();
    xhr.open("POST", "save_data.php", true);
    xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");

    // Gửi yêu cầu lưu dữ liệu
    xhr.onload = function () {
      if (xhr.status === 200) {
        alert(xhr.responseText);
      } else {
        alert("Lỗi khi lưu dữ liệu.");
      }
    };

    xhr.send("save=true");
  }
}
      
    </script>
  </body>
</html>
