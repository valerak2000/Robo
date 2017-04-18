#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h> 
#include <Wire.h> // библиотека работы последовательного порта

// GPIO, куда подключено реле
int rele1 = 14;
int rele2 = 16;
bool PowerRele1  = false;
bool PowerRele2  = false;

// Имя хоста
const char* host = "test";
const char* hostWiFi = "testESP";

// параметры вашей WiFi сети. (Те что всегда вводите на тел. и планшете)
const char* ssid = "BtlabKR";
const char* password = "q2w3e4r5t";

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200); // включим последовательный порт? скорость 115200
  // инициализация выводов.
  pinMode(LED_BUILTIN, OUTPUT);  // Светодиод на ESP8266 ESP-12F (вывод инвертирован)
  pinMode(D6, OUTPUT); // 12 LED 3-х цв. (зеленый)
  pinMode(D7, OUTPUT); // 13 LED 3-х цв. (синий)
  pinMode(D10, OUTPUT); // 15 LED 3-х цв. (красный)
  analogWrite(LED_BUILTIN, 1023);
  analogWrite(D6, LOW);
  analogWrite(D7, LOW);
  analogWrite(D10, LOW);

  //pinMode(rele1, OUTPUT);
  //digitalWrite(rele1, PowerRele1);
  //pinMode(rele2, OUTPUT);
  //digitalWrite(rele2, PowerRele2);

  WiFi.hostname(hostWiFi);
  WiFi.mode(WIFI_STA);  //WIFI_AP_STA
  WiFi.begin(ssid, password);
  // Подлючение к WiFi
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    // Запускаем сервер
    //MDNS.begin(host);
    //server.on("/", HTTP_handleRoot);
    //server.onNotFound(HTTP_handleRoot);
    //server.begin();       
  }
}

void loop() {
  //server.handleClient();
  // -127 (weak) to -1dB (strong)
  int32_t rssi = WiFi.RSSI();
  int32_t led_power = 1023;
  byte Red, Green, Blue;
  if (rssi < 0) {
    if (rssi > -40) {
      Serial.println("Signal strength is good! RSSI:" + String(rssi) + "dB.");
      Red = 0;
      Green = 255;
      Blue = 0;
      led_power = 0;
    } 
    else {
      if (rssi > -60) {
        Serial.println("Signal strength is OK - RSSI:" + String(rssi) + "dB.");
        Red = 128;
        Green = 196;
        Blue = 0;
        led_power = 100;
      }
      else {
        if (rssi > -80) {
          Serial.println("Signal strength is problematic - RSSI:" + String(rssi) + "dB.");
          Red = 255;
          Green = 96;
          Blue = 0;
          led_power = 900;
        }
        else {
          Serial.println("Signal strength is almost unusable - RSSI:" + String(rssi) + "dB.");
          Red = 255;
          Green = 0;
          Blue = 0;
          led_power = 1023;
        }
      }
    }
  } 
  else {
    Red = 255;
    Green = 0;
    Blue = 0;
    led_power = 1023;
  }

  analogWrite(LED_BUILTIN, led_power);   // LED 
  analogWrite(D6, Green);   // LED 3-х цв. (зеленый)
  analogWrite(D7, Blue);   // LED 3-х цв. (синий)
  analogWrite(D10, Red);   // LED 3-х цв. (красный)

  delay(500);
}

void HTTP_handleRoot(void) {
  bool statrele1 = false;
  bool statrele2 = false;
  // Реле 1
  if (server.hasArg("statrele1")) {
    if (strncmp(server.arg("statrele1").c_str(), "1", 1) == 0) {
      statrele1 = true;
      Serial.println("Rele1 ON"); // читаем значение с фоторезистора и выводи его в порт
    }
    else {
      Serial.println("Rele1 OFF"); // читаем значение с фоторезистора и выводи его в порт
    }
  }
  else {
     statrele1 = PowerRele1;
  }
    // Реле 2
    if (server.hasArg("statrele2")) {
      if (strncmp(server.arg("statrele2").c_str(), "1", 1) == 0) {
        statrele2 = true;
        Serial.println("Rele2 ON"); // читаем значение с фоторезистора и выводи его в порт
      }
      else {
        Serial.println("Rele1 OFF"); // читаем значение с фоторезистора и выводи его в порт
      }
  }
  else {
     statrele2 = PowerRele2;
  }
  
  // Формируем ШТМЛ страницу
  String out = "";

  out =
"<html>\
  <head>\
    <meta charset=\"utf-8\" />\
    <title>Управление Реле</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
";

// Для реле 1
  if (statrele1) {
      out += "\
    <hr width=100 size=30 align=left color=red><h2><a href=\"/?statrele1=0\">OFF</a></h2>\
    ";
  }
  else {
      out += "\
    <hr width=100 size=30 align=left color=green><h2><a href=\"/?statrele1=1\">ON</a></h2>\
    ";            
  }

  // Для реле 2
  if (statrele2) {
      out += "\
    <hr width=100 size=30 align=left color=red><h2><a href=\"/?statrele2=0\">OFF</a></h2>\
    ";
  }
  else {
      out += "\
    <hr width=100 size=30 align=left color=green><h2><a href=\"/?statrele2=1\">ON</a></h2>\
    ";            
  }

   // Общее 
   out += "\
  </body>\
</html>";
  server.send(200, "text/html", out);
      
  // Для реле 1
  if (statrele1 != PowerRele1) {
    PowerRele1 = statrele1;
    digitalWrite(rele1, PowerRele1);
  }
  // Для реле 2
  if (statrele2 != PowerRele2) {
    PowerRele2 = statrele2;
    digitalWrite(rele2, PowerRele2);
  }
}

/*
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
 
const char* ssid = "testAP";
const char* password = "12345678";
MDNSResponder mdns;

ESP8266WebServer server(80);

const int led = 13;

void handleRoot() {
  server.send(200, "text/plain", 
    "hello from esp8266!) \n/on: to tuen LED ON \n/off: to tuen LED OFF \n");

}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}
 
void setup(void){
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
  
  server.on("/", handleRoot);

  server.on("/on", [](){
    digitalWrite(BUILTIN_LED, LOW);
    server.send(200, "text/plain", "LED ON");
  });
  
  server.on("/off", [](){
    digitalWrite(BUILTIN_LED, HIGH);
    server.send(200, "text/plain", "LED OFF");
  });

  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("HTTP server started");

  //delay a moment, 
  //for terminal to receive inf, such as IP address
  delay(1000);
  Serial.end();
  pinMode(BUILTIN_LED, OUTPUT);
}
 
void loop(void){
  server.handleClient();
}  
 */
