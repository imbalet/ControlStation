#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

#ifdef ESP32
#include <WiFi.h>
#endif

#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include "html.h"

const char *ssid = "pen is";
const char *password = "12345678";

#define DEF_PAD_DATA "%72/0a255255255255/0b000000000000000000/1a255255255255/1b000000000000000000/"
#define UART_TIMEOUT 500

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

String msg = "";

bool connected = 0;
uint32_t timer = 0;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
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

void setup()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
  }

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.begin(115200, SERIAL_8N1);

  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();
}

uint32_t send_timer = 0;

void loop()
{
  server.handleClient();
  webSocket.loop();

  yield();
  if (millis() - send_timer >= 50)
  {
    if (millis() - timer >= UART_TIMEOUT)
    {
      Serial.print(DEF_PAD_DATA);
      Serial.print('\r');
    }
    else
    {
      Serial.print(msg);
      Serial.print('\r');
    }
    send_timer = millis();
  }
}