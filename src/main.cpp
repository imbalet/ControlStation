#include "main.h"
#include "web.h"
#include <cstdlib>
#include <cstring>

const int coords_recieved = 49;


String msg;

bool connected = 0;
uint32_t timer = 0;

uint32_t mosfet_timer = 0;
void setup()
{
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);

  #ifndef ESP8266
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(const_cast<CRGB*>(leds), NUM_LEDS);
  FastLED.setBrightness(10);//0-255
  FastLED.show();
  #endif

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
    display.clearDisplay();
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
int recieve_data = 0;
uint8_t coords_flag = 0, got_coords = 0;

void loop()
{
  if( Serial.available() ){
    recieve_data = Serial.read();
    Serial.print(recieve_data);
    if (got_coords && recieve_data == coords_recieved){
      coords_flag = 2;
      got_coords = 0;
    }
  }


//   if( Serial.available() ){
//     position = Serial.read();
//     switch(position){
//       case 1:
//         FastLED.addLeds<WS2812B, DATA_PIN, GRB>(const_cast<CRGB*>(num1), NUM_LEDS);
//         FastLED.setBrightness(100);//0-255
//         FastLED.show();
//         break;
//       case 2:
//         FastLED.addLeds<WS2812B, DATA_PIN, GRB>(const_cast<CRGB*>(num2), NUM_LEDS);
//         FastLED.setBrightness(100);//0-255
//         FastLED.show();
//         break;
//       case 3:
//         FastLED.addLeds<WS2812B, DATA_PIN, GRB>(const_cast<CRGB*>(num3), NUM_LEDS);
//         FastLED.setBrightness(100);//0-255
//         FastLED.show();
//         break;
//     }
// }


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