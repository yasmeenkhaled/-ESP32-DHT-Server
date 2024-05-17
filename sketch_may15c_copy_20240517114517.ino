#include "WiFi.h"
#include "AsyncTCP.h"//tcp socket
#include "ESPAsyncWebServer.h" // http server
#include <Adafruit_Sensor.h>  
#include <DHT.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* wifiSSID = "Marvel";
const char* wifiPassword = "01005180055khaledQotb1";
String weatherServer = "api.openweathermap.org";
String city = "Cairo,eg";
String apiKey = "05659c21bdf4e171a05529c92c709588";

#define DHTPIN 26
#define DHTTYPE    DHT22
DHT dht(DHTPIN, DHTTYPE);

AsyncWebServer server(80);

String readDHTTemperature() {
  float temperature = dht.readTemperature();
  if (isnan(temperature)) {    
    Serial.println("Failed to read from temperature sensor!");
    return "--";
  }
  else {
    Serial.println(temperature);
    return String(temperature);
  }
}

String readDHTHumidity() {
  float humidity = dht.readHumidity();
  if (isnan(humidity)) {
    Serial.println("Failed to read from humidity sensor!");
    return "--";
  }
  else {
    Serial.println(humidity);
    return String(humidity);
  }
}

float webTemperature = 0.0;
float webHumidity = 0.0;
float temperatureAccuracy = 0.0;
float humidityAccuracy = 0.0;

void fetchWeatherData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://" + String(weatherServer) + "/data/2.5/weather?q=" + city + "&units=metric&appid=" + apiKey;

    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);

        webTemperature = doc["main"]["temp"];
        webHumidity = doc["main"]["humidity"];

        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();
        
        temperatureAccuracy = (1 - abs(temperature - webTemperature) / webTemperature) * 100;
        humidityAccuracy = (1 - abs(humidity - webHumidity) / webHumidity) * 100;

        Serial.print("Website temperature: ");
        Serial.print(webTemperature);
        Serial.println(" °C");
        Serial.print("Website humidity: ");
        Serial.print(webHumidity);
        Serial.println("%");
        Serial.print("Temperature Accuracy: ");
        Serial.println(temperatureAccuracy);
        Serial.print("Humidity Accuracy: ");
        Serial.println(humidityAccuracy);
      }
    }
    http.end();
  }
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
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
  </style>
</head>
<body>
  <h2>ESP32 WEATHER </h2>
<div style="
  display:flex;
  justify-content:center;
">
<div>

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
    <sup class="units">&percnt;</sup>
  </p></div>
  <div>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Google Temp</span> 
    <span id="webTemperature">%WEBTEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Google Humidity</span>
    <span id="webHumidity">%WEBHUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p>
 
 
</div>

</div>
 <p>
    <i class="fas fa-chart-line" style="color:#059e8a;"></i> 
    <span class="dht-labels">TemperatureAccuracy</span> 
    <span id="temperatureAccuracy">%TACCURACY%</span>
    <sup class="units">%</sup>
  </p>
   <p>
    <i class="fas fa-chart-line" style="color:#059e8a;"></i> 
    <span class="dht-labels">HumidityAccuracy</span> 
    <span id="humidityAccuracy">%HACCURACY%</span>
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

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperatureAccuracy").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperatureAccuracy", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidityAccuracy").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidityAccuracy", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>
)rawliteral";

String processor(const String& var){
  if(var == "TEMPERATURE"){
    return readDHTTemperature();
  }
  else if(var == "HUMIDITY"){
    return readDHTHumidity();
  }
  else if(var == "WEBTEMPERATURE"){
    return String(webTemperature);
  }
  else if(var == "WEBHUMIDITY"){
    return String(webHumidity);
  }
  else if(var == "TACCURACY"){
    return String(temperatureAccuracy);
  }
  else if(var == "HACCURACY"){
    return String(humidityAccuracy);
  }
  return String();
}

void setup(){
  Serial.begin(115200);
  dht.begin();
  WiFi.begin(wifiSSID, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor); //http status code
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });
  server.on("/webTemperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(webTemperature).c_str());
  });
  server.on("/webHumidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(webHumidity).c_str());
  });
  server.on("/temperatureAccuracy", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(temperatureAccuracy).c_str());
  });
  server.on("/humidityAccuracy", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(humidityAccuracy).c_str());
  });
  server.begin();
  fetchWeatherData(); 
}

void loop(){
}
  