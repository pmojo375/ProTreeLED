#ifndef webSockets_h
#define webSockets_h

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ledFunctions.h>
#include <FastLED.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <time.h>
#include <Arduino.h>

extern AsyncWebServer server;
extern AsyncWebSocket ws_logs;        // WebSocket for logs
extern AsyncWebSocket ws_chart;      // WebSocket for mic chart
extern AsyncWebSocket ws_control;  // WebSocket for your other page

extern int fpsVariability;
extern int fps;
extern bool inc_gHueState;
extern uint8_t fadeAmount;
extern int mode;
extern CRGB color1;
extern CRGB color2;
extern CRGB color3;
extern CRGB color4;
extern uint8_t brightness;
extern bool setBrightness;

// Function to broadcast log messages;
void broadcastLog(String message);

void sendMessageToClients(const String &message);

void onWebSocketControlEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

void configWS();

#endif