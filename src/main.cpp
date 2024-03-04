#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

#ifdef ESP32
#include <WiFi.h>
#endif

#include "ESP8266WebServer.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "WebSocketsServer.h"
#include "html.h"

#define STA 1 // 1 for STA, 0 - AP

const char *ap_ssid = "ROBOT";
const char *ap_password = "12345678";

#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(128, 64, &Wire, -1);

const char *ssid = "wintest";
const char *password = "12345678";

#define DEF_PAD_DATA "%080/0a256255255255/0b000000000000000000/1a255255255255/1b000000000000000000/097/"
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
uint32_t mosfet_timer = 0;
void setup()
{
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);
  
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("No connection((");
  display.display();

  #if STA

  WiFi.begin(ssid, password);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Connecting.......");
  display.display();

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    display.setCursor(0,1);
    display.print(millis());
    display.display();
  }

  #else
  
  WiFi.softAP(ap_ssid, ap_password);
  
  #endif

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.begin(115200, SERIAL_8N1);

  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(WiFi.localIP());
  display.println((WiFi.softAPIP()));
  display.display();
  mosfet_timer = millis();

}

uint32_t send_timer = 0;

void loop()
{
  if (millis() - mosfet_timer >= 5000){
    digitalWrite(2, 1);
  }
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