// #include <ESP8266WiFi.h>
// #include <WiFiClient.h>
// #include <ESP8266WebServer.h>
// #include "html.h"

// const char* ssid = "вифи 412/1";
// const char* pass = "FE4121FE";
// ESP8266WebServer server(80);





// void data(){
//   Serial.println(server.args());
//     if (server.hasArg("count")){
//     Serial.println("Found cookie: ");
//     Serial.println(ESP.getFreeHeap());
//     }
//   for (int i = 0; i < server.args(); i++){
//     Serial.print(server.argName(i));
//     Serial.println(server.arg(server.argName(i)));
//   }
//   server.send(200);

// }
// void root(){
//   server.send(200, "text/html", webpage);
// }

// void setup() {
// server.on("/data", data);
// server.on("/", root);

//   server.begin();
//   Serial.begin(115200);
//   WiFi.begin(ssid, pass);
//   for (int i = 0; i < 150; i++) {
//     if (WiFi.status() == WL_CONNECTED) {
//       Serial.println("");
//       Serial.print("Connected to ");
//       Serial.print("IP address: ");
//       Serial.println(WiFi.localIP());
//       break;
//     }
//     Serial.print(".");
//     delay(100);
//   }
// }
// long timer = millis();
// void loop() {
// server.handleClient();
// }

// Import required libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "html.h"

// Replace with your network credentials
const char* ssid = "вифи 412/1";
const char* password = "FE4121FE";

bool ledState = 0;
const int ledPin = 2;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void notifyClients() {
  ws.textAll(String(ledState));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    Serial.print("                              ");
    Serial.print('\r');
    //Serial.print((char*)data);
    Serial.print("   ");
    Serial.print(ESP.getFreeHeap());
    
    //notifyClients();
    //if (strcmp((char*)data, "toggle") == 0) {

      
    //}
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
      case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
      case WS_EVT_PONG:
      case WS_EVT_ERROR:
        break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (ledState){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  //pinMode(ledPin, OUTPUT);
  ///digitalWrite(ledPin, LOW);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Start server
  server.begin();
}

void loop() {
  ws.cleanupClients();
  yield();
  //Serial.println(ESP.getFreeHeap());
}