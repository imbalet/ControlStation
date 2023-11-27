#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include "html.h"

const char* ssid = "вифи 412/1";
const char* password = "FE4121FE";

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

struct gamepad{
  int axes [4];
  bool buttons [20];
  //bool control; //0-movement 1-manipulator
};

gamepad pad0, pad1;

gamepad pads [2] = {pad0, pad1};

String msg;
String prevMsg;


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

switch(type) {
  case WStype_DISCONNECTED:
    //Serial.printf("[%u] Disconnected!\n", num);
    Serial.println("Disconnected");
    break;
  case WStype_CONNECTED:
  {
    IPAddress ip = webSocket.remoteIP(num);
    //Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
    // send message to client
    webSocket.sendTXT(num, "Connected");
  }
    break;
  case WStype_TEXT:
    msg = ((char*) payload);
    //Serial.printf("[%u] get Text: %s\n", num, payload);
    // send message to client
    // webSocket.sendTXT(num, "message here");
    // send data to all connected clients
    // webSocket.broadcastTXT("message here");
    break;
  case WStype_BIN:
    Serial.println("Binary");
    //Serial.printf("[%u] get binary length: %u\n", num, length);
    //hexdump(payload, length);
    // send message to client
    // webSocket.sendBIN(num, payload, length);
    break;
}

}



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


void handleRoot(){
  server.send(200, "text/html", index_html);
}



void setup(){
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();
}

long timer;

void loop() {
  server.handleClient();
  webSocket.loop();
  
  yield();

  if (millis() - timer >= 100){
    prevMsg = msg;
    Serial.print("                              ");
    Serial.print('\r');
    convertData((char*)prevMsg.c_str());
    Serial.print("   ");
    Serial.print(pads[0].axes[0]);
        Serial.print("   ");
    Serial.print(pads[1].axes[0]);
    Serial.print("   ");
    Serial.print(ESP.getFreeHeap());
    timer = millis();
  }
}