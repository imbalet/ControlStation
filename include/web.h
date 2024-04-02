#include "main.h"
#include "html.h"

#ifndef WEB
#define WEB


extern uint32_t timer;
extern String msg;
extern uint8_t coords_flag;
extern uint8_t got_coords;
extern bool connected;


ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);



void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch(type)
  {
  case WStype_DISCONNECTED:
    connected = 0;
    // Serial.println("Disconnected");
    break;
  case WStype_CONNECTED:
  {
    connected = 1;
    IPAddress ip = webSocket.remoteIP(num);
    webSocket.sendTXT(num, "Connected");
  }
  break;
  case WStype_TEXT:
    timer = millis();
    msg = ((String)((char *)payload)).substring(1);
    if ((char)payload[0] == '1')
    {
      webSocket.sendTXT(num, "got");
    }

    if ((char)payload[6] == '$'){
      got_coords = 1;
      if (coords_flag){
        webSocket.sendTXT(num, "cord");
        coords_flag = 0;
      }
    }
    else{
      got_coords = 0;
      coords_flag = 0;
    }
    break;
  case WStype_BIN:
    // Serial.println("Binary");
    break;
  }
}

void handleRoot()
{
  server.send(200, "text/html", index_html);
}

#endif