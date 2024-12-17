#ifndef webSockets_h
#define webSockets_h

#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <ledFunctions.h>
#include <FastLED.h>
#include <LittleFS.h>

extern AsyncWebServer server(80);
extern AsyncWebSocket ws_logs("/ws_logs");        // WebSocket for logs
extern AsyncWebSocket ws_chart("/ws_chart");      // WebSocket for mic chart
extern AsyncWebSocket ws_control("/ws_control");  // WebSocket for your other page

extern int fpsVariability;
extern int fps;
extern bool inc_gHueState;
extern uint8_t fadeAmount;
extern int mode;
extern CRGB color1 = CRGB::DarkGreen;
extern CRGB color2 = CRGB::Red;
extern CRGB color3 = CRGB::Blue;
extern CRGB color4 = CRGB::WhiteSmoke;
extern uint8_t brightness;
extern bool setBrightness;

void sendMessageToClients(const String &message);

void onWebSocketControlEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

void configWS();

#endif