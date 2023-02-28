#include <Arduino.h>
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
WebSocketsClient webSocket;

////////////////WIFI////////////////////
const char *ssid     = "!!!!WIFINAME!!!!";
const char *password = "!!!!WIFIPASSWORD!!!!";
const char *websocketURL = "!!!!WEBSocketIP!!!!";
const int websocketPORT = 8889;
bool connected = false;
//#define DEBUG_SERIAL //Serial
////////////////RGB////////////////////
#define LED_PIN     D7
#define NUM_LEDS    300
#define brightKnob  A0
#define pwrSwitch   D2
#define pwrLED      D1
const int pwrLEDBrightness = 50;
CRGB leds[NUM_LEDS];



int disconnectedCount = 0;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:           
            if (disconnectedCount > 5) {
              FastLED.clear(true);
              delay(2000);
            } else {
              disconnectedCount++;
            }
            connected = false;
            break;
        case WStype_CONNECTED: 
            disconnectedCount = 0;
            connected = true;
            break;
        case WStype_TEXT:
            rgbMagic(payload);
            break;
        case WStype_BIN:
            hexdump(payload, length);
            break;
        case WStype_PING:
            break;
        case WStype_PONG:
            break;
    }
 }


int rgbMagic(uint8_t * payload) {
    String payloadString = (char *) payload;
    FastLED.clear(false);
    DynamicJsonDocument doc(30024);
JsonArray array;
    DeserializationError error = deserializeJson(doc, payloadString);
    array = doc.as<JsonArray>();
    int index = 0;
    for (int i = 0; i < array.size(); i += 3) {
        leds[index++] = CRGB(int(doc[i]),int(doc[i+1]),int(doc[i+2]));
        leds[index++] = CRGB(int(doc[i]),int(doc[i+1]),int(doc[i+2]));
        if (int(i/3) > NUM_LEDS) {
          return 1;
        }
     }

    FastLED.show();
    return 0;


}

 
void setup() {
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(255);
    FastLED.clear(true);
    pinMode(pwrLED, OUTPUT);
    pinMode(pwrSwitch, INPUT_PULLUP);

  
    for(uint8_t t = 3; t > 0; t--) {
        delay(1000);
    }
 
    WiFi.begin(ssid, password);
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
 
    while ( WiFi.status() != WL_CONNECTED ) {
      delay ( 500 );
    }

    webSocket.begin(websocketURL, websocketPORT, "/");

    webSocket.onEvent(webSocketEvent);
}
 
 
 
void loop() {
    if (digitalRead(pwrSwitch) == LOW) {
      FastLED.clear(true);
      FastLED.show();
      digitalWrite(pwrLED, LOW); 
    } else {
      analogWrite(pwrLED, pwrLEDBrightness );
      int val = map(analogRead(brightKnob), 2, 1023, 0, 255); 
      FastLED.setBrightness(val);
      FastLED.show();
      webSocket.loop();
    }     
}
