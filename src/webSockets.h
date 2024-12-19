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

// Initialize an empty JSON document
extern JsonDocument doc2;

// Create an empty JsonArray
extern JsonArray group;

// Function to broadcast log messages;
void broadcastLog(String message);

void sendMessageToClients(const String &message);

void onWebSocketControlEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

void setupWebSockets();

#endif