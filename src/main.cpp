#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "html.h"

const char* ssid = "вифи 412/1";
const char* password = "FE4121FE";

bool ledState = 0;
const int ledPin = 2;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

struct gamepad{
  int axes [4];
  bool buttons [20];
  //bool control; //0-movement 1-manipulator
};

gamepad pad0, pad1;

gamepad pads [2] = {pad0, pad1};

void notifyClients() {
  ws.textAll("OK");
}

// void convertData(char* data){ //        0a001001001001/0b0000000000000000000/a001001001001/0b000000000000000000/
//   //gamepad* structPtr;
//   bool now; //1 - axes, 0 - buttons
//   uint8_t pad;
//   for (int i; data[i] != '\0'; i++){
//     if(data[i] == 'a' || data[i] == 'b'){
//       now = data[i] == 'a' ? 1 : 0;
//       pad = data[i-1] - 48;
//       now = 1;
//       continue;
//     }
//     for (int j = 0; data[i] != '/';){
//       if(now){
//         pads[pad].axes[j] = (data[i] - 48) * 100 + (data[i+1] - 48) * 10 + (data[i+2] - 48); 
//         j++;
//         i += 3;
//       }
//       else{
//         pads[pad].buttons[j] = data[i] - 48;
//         j++;
//         i++;
//       }
//     }
//   }
// }

void convertData(char* data){ //        0a001001001001/0b0000000000000000000/a001001001001/0b000000000000000000/
  Serial.print(data[1]);
  bool now; //1 - axes, 0 - buttons
  uint8_t pad;
  if(!(data[1] == 'a' || data[1] == 'b')){
    // Serial.print("here0");
    // Serial.print(data[1] != 'a' );
    // Serial.print(data[1] != 'b');
    return;
  } 
  //Serial.print("here");
  for (int i = 1; data[i] != '\0'; i++){
      if (data[i] == '\0') break;
    if(data[i] == 'a' || data[i] == 'b'){
      now = data[i] == 'a' ? 1 : 0;
      pad = data[i-1] - 48;
      continue;
    }
    //Serial.print("here1");
    for (int j = 0; data[i-1] != '/';){
        if (data[i] == '/') break;
      if(now){
        pads[pad].axes[j] = (data[i] - 48) * 100 + (data[i+1] - 48) * 10 + (data[i+2] - 48); 
        j++;
        i += 3;
      }
      else if(!now){
        pads[pad].buttons[j] = data[i] - 48;
        j++;
        i++;
      }
    }
  }
  //Serial.print(pads[0].axes[0]);
}


void printintarr(int* arr, int len){
    for (int i = 0; i<len; i++){
        Serial.println(arr[i]);
    }
}


void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    Serial.print("                              ");
    Serial.print('\r');
    convertData((char*)data);
    //printintarr(pads[0].axes,4);
    // Serial.print((char*)data);
    Serial.print("   ");
    Serial.print(pads[0].axes[0]);
        Serial.print("   ");
    Serial.print(pads[1].axes[0]);
    Serial.print("   ");
    Serial.print(ESP.getFreeHeap());
    
    //notifyClients();
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


void setup(){
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  initWebSocket();
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.begin();
}

void loop() {
  ws.cleanupClients();
  yield();
}