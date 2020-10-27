/*********
  ĐTAN 20193
  Họ tên: Nguyễn Bá Khởi
  MSSV: 20146406
*********/

// Import required libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

//Chèn mật khẩu và SSID sử dụng
const char* ssid = "Ahaha Aloha";
const char* password = "123456a@";

// Cài đặt LED GPIO
const int ledPin = 4;
// Stores LED state
String ledState;

#define DHTPIN 14     // Chân kỹ thuật số kết nối với cảm biến DHT

// Cảm biến sử dụng là DHT11
#define DHTTYPE    DHT11     // DHT 11


DHT dht(DHTPIN, DHTTYPE);

// Nhiệt độ và độ ẩm hiện tại, được cập nhật trong loop ()
float t = 0.0;
float h = 0.0;

// Tạo đối tượng AsyncWebServer trên cổng 80
AsyncWebServer server(80);

//Sử dụng "unsigned long" cho các biến giữ thời gian
// Giá trị sẽ nhanh chóng trở nên quá lớn để một số nguyên có thể lưu trữ
unsigned long previousMillis = 0;    // sẽ lưu trữ lần cuối DHT được cập nhật

// Cập nhật việ đọc DHT sau mỗi 10 giây
const long interval = 10000;  

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Final Project</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    .ten {
      text-align: left;
      font-size: 0.5rem;
    }
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
    .button {
  display: inline-block;
  background-color: #008CBA;
  border: none;
  border-radius: 4px;
  color: white;
  padding: 16px 40px;
  text-decoration: none;
  font-size: 30px;
  margin: 2px;
  cursor: pointer;
}
.button2 {
  background-color: #f44336;
}
  </style>
</head>
<body>
  <div class="ten">
    <p>Full name: Nguyen Ba Khoi</p>
    <p>ID Student  : 20146406</p>
    
  </div>
  <h2>ESP8266 DHT Server</h2>
  <p>GPIO state<strong> %STATE%</strong></p>
  <p>
    <a href="/on"><button class="button">ON</button></a>
    <a href="/off"><button class="button button2">OFF</button></a>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if(digitalRead(ledPin)){
      ledState = "ON";
    }
    else{
      ledState = "OFF";
    }
    Serial.print(ledState);
    return ledState;
  }else if(var == "TEMPERATURE"){
    return String(t);
  }
   else if(var == "HUMIDITY"){
    return String(h);
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  dht.begin();
  
  //Kết nối tới Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }

  // In ra địa chỉ IP của ESP8266
  Serial.println(WiFi.localIP());

  // Định tuyến cho trang gốc/trang web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  
  //Định tuyến để đặt GPIO thành HIGH
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, HIGH);    
    request->send_P(200, "text/html", index_html, processor);
  });
  
  //Định hướng đặt GPIO thành LOW
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, LOW);    
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(h).c_str());
  });

  //Bắt đầu chạy server
  server.begin();
}
 
void loop(){  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    //Lưu lần cuối cùng bạn cập nhật các giá trị DHT
    previousMillis = currentMillis;
    //Đọc nhiệt độ dưới dạng độ C (mặc định)
    float newT = dht.readTemperature();
    // Đọc nhiệt độ dưới dạng Fahrenheit (isFahrenheit = true)
    //float newT = dht.readTemperature(true);
    //Nếu nhiệt độ đọc không thành công, không thay đổi giá trị t
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      t = newT;
      Serial.println(t);
    }
    //Đọc độ ẩm
    float newH = dht.readHumidity();
    //Nếu độ ẩm không đọc được, không thay đổi giá trị h 
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      h = newH;
      Serial.println(h);
    }
  }
}
