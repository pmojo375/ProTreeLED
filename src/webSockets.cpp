#include <webSockets.h>

// Create an AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws_logs("/ws_logs");        // WebSocket for logs
AsyncWebSocket ws_chart("/ws_chart");      // WebSocket for mic chart
AsyncWebSocket ws_control("/ws_control");  // WebSocket for your other page

// Handle incoming data
JsonDocument jsonDoc;
std::vector<int> group1 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
std::vector<int> group2 = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
std::vector<int> group3 = {20, 21, 22, 23, 24, 25, 26, 27, 28, 29};
std::vector<int> group4 = {30, 31, 32, 33, 34, 35, 36, 37, 38, 39};
std::vector<int> group5 = {40, 41, 42, 43, 44, 45, 46, 47, 48, 49};
JsonArray group;

// Function to broadcast log messages
void broadcastLog(String message) {
  // Get current time in date time format
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  char timeString[30];
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
  // Send to log WebSocket clients
  String logMessage = timeString + String(" - ") + message;
  ws_logs.textAll(logMessage);
  Serial.println(logMessage);  // Also print to Serial
}

void sendMessageToClients(const String &message) {
  ws_control.textAll(message);
}

// Handle WebSocket events
void onWebSocketControlEvent(AsyncWebSocket *server,
                             AsyncWebSocketClient *client, AwsEventType type,
                             void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("WebSocket client connected");
    sendMessageToClients("{\"type\":\"Mode\",\"value\":\"" + String(mode) +
                         "\"}");
    sendMessageToClients("{\"type\":\"Increment gHue\",\"value\":\"" +
                         String(inc_gHueState) + "\"}");
    sendMessageToClients("{\"type\":\"Palette\",\"value\":\"" +
                         String(getPalette()) + "\"}");
    sendMessageToClients("{\"type\":\"Color 1\",\"value\":\"" +
                         CRGBToHex(color1) + "\"}");
    sendMessageToClients("{\"type\":\"Color 2\",\"value\":\"" +
                         CRGBToHex(color2) + "\"}");
    sendMessageToClients("{\"type\":\"Color 3\",\"value\":\"" +
                         CRGBToHex(color3) + "\"}");
    sendMessageToClients("{\"type\":\"Color 4\",\"value\":\"" +
                         CRGBToHex(color4) + "\"}");
    sendMessageToClients("{\"type\":\"FPS\",\"value\":\"" + String(fps) +
                         "\"}");
    sendMessageToClients("{\"type\":\"Fade Amount\",\"value\":\"" +
                         String(fadeAmount) + "\"}");
    sendMessageToClients("{\"type\":\"Brightness\",\"value\":\"" +
                         String(brightness) + "\"}");
    sendMessageToClients("{\"type\":\"FPS Variability\",\"value\":\"" +
                         String(fpsVariability) + "\"}");
    sendMessageToClients("{\"type\":\"Twinkle Speed\",\"value\":\"" +
                         String(twinkleSpeed) + "\"}");
    sendMessageToClients("{\"type\":\"Twinkle Density\",\"value\":\"" +
                         String(twinkleDensity) + "\"}");
    sendMessageToClients("{\"type\":\"Cool Like\",\"value\":\"" +
                         String(coolLikeIncandescentEn) + "\"}");
    sendMessageToClients("{\"type\":\"Auto Bg\",\"value\":\"" +
                         String(autoSelectBackgroundColor) + "\"}");
    sendMessageToClients("{\"type\":\"Brightness Audio\",\"value\":\"" +
                         String(setBrightness) + "\"}");
    sendMessageToClients("{\"type\":\"Start Range\",\"value\":\"" +
                         String(startRange) + "\"}");
    sendMessageToClients("{\"type\":\"End Range\",\"value\":\"" +
                         String(endRange) + "\"}");
    sendMessageToClients("{\"type\":\"Aurora Speed\",\"value\":\"" +
                         String(auroraSpeed) + "\"}");
    sendMessageToClients("{\"type\":\"Aurora Intensity\",\"value\":\"" +
                         String(auroraIntensity) + "\"}");
    sendMessageToClients("{\"type\":\"Aurora Wave Count\",\"value\":\"" +
                         String(auroraWaveCount) + "\"}");
    sendMessageToClients("{\"type\":\"Aurora Color Range\",\"value\":\"" +
                         String(auroraColorRange) + "\"}");
    sendMessageToClients("{\"type\":\"Ring Speed\",\"value\":\"" +
                         String(ringSpeed) + "\"}");
    sendMessageToClients("{\"type\":\"Ring Intensity\",\"value\":\"" +
                         String(ringIntensity) + "\"}");
    sendMessageToClients("{\"type\":\"Tree Height\",\"value\":\"" +
                         String((int)treeHeight) + "\"}");
    sendMessageToClients("{\"type\":\"Tree Taper Ratio\",\"value\":\"" +
                         String((int)(treeTaperRatio * 100)) + "\"}");
    sendMessageToClients("{\"type\":\"Test Ring Number\",\"value\":\"" +
                         String(testRingNumber) + "\"}");
    sendMessageToClients("{\"type\":\"Bottom Ring LEDs\",\"value\":\"" +
                         String(bottomRingLEDs) + "\"}");
    sendMessageToClients("{\"type\":\"Middle Ring LEDs\",\"value\":\"" +
                         String(middleRingLEDs) + "\"}");
    sendMessageToClients("{\"type\":\"Top Ring LEDs\",\"value\":\"" +
                         String(topRingLEDs) + "\"}");
    sendMessageToClients("{\"type\":\"Top Zone Height\",\"value\":\"" +
                         String((int)(topZoneHeight * 100)) + "\"}");
    sendMessageToClients("{\"type\":\"Breathing Rate\",\"value\":\"" +
                         String(breathingRate) + "\"}");
    sendMessageToClients("{\"type\":\"Breathing Variability\",\"value\":\"" +
                         String(breathingVariability) + "\"}");
    sendMessageToClients("{\"type\":\"Breath Hold Time\",\"value\":\"" +
                         String(breathHoldTime) + "\"}");
    sendMessageToClients("{\"type\":\"Audio Min Amplitude\",\"value\":\"" +
                         String((int)audioMinAmplitude) + "\"}");
    sendMessageToClients("{\"type\":\"Audio Max Amplitude\",\"value\":\"" +
                         String((int)audioMaxAmplitude) + "\"}");
    sendMessageToClients("{\"type\":\"Audio Smoothing\",\"value\":\"" +
                         String((int)(audioSmoothing * 100)) + "\"}");
    sendMessageToClients("{\"type\":\"Audio Color Speed\",\"value\":\"" +
                         String(audioColorSpeed) + "\"}");
    sendMessageToClients("{\"type\":\"Audio Wave Speed\",\"value\":\"" +
                         String(audioWaveSpeed) + "\"}");
    sendMessageToClients("{\"type\":\"Auto Brightness Enabled\",\"value\":\"" +
                         String(autoBrightnessEnabled ? "true" : "false") + "\"}");
    sendMessageToClients("{\"type\":\"Auto Brightness Min Brightness\",\"value\":\"" +
                         String(autoBrightnessMinBrightness) + "\"}");
    sendMessageToClients("{\"type\":\"Auto Brightness Max Brightness\",\"value\":\"" +
                         String(autoBrightnessMaxBrightness) + "\"}");
    sendMessageToClients("{\"type\":\"Auto Brightness Min Amplitude\",\"value\":\"" +
                         String((int)autoBrightnessMinAmplitude) + "\"}");
    sendMessageToClients("{\"type\":\"Auto Brightness Max Amplitude\",\"value\":\"" +
                         String((int)autoBrightnessMaxAmplitude) + "\"}");
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("WebSocket client disconnected");
  } else if (type == WS_EVT_DATA) {
    deserializeJson(jsonDoc, data);  // Parse the JSON data

    String type = jsonDoc["type"];    // Get the type of message
    String value = jsonDoc["value"];  // Get the value

    broadcastLog("Type: " + type);
    broadcastLog("Value: " + value);

    // Act based on the type of message
    if (type == "Mode") {
      mode = value.toInt();
    } else if (type == "Increment gHue") {
      if (value == "true") {
        inc_gHueState = true;
      } else {
        inc_gHueState = false;
      }
    } else if (type == "Group1") {
      group = jsonDoc["value"].as<JsonArray>();
      // log the group
      group1.clear();
      for (JsonVariant led : group) {
        if (led.is<int>()) {
          group1.push_back(led.as<int>());
        }
      }
    } else if (type == "Group2") {
      group = jsonDoc["value"].as<JsonArray>();
      // log the group
      group2.clear();
      for (JsonVariant led : group) {
        if (led.is<int>()) {
          group2.push_back(led.as<int>());
        }
      }
    } else if (type == "Group3") {
      group = jsonDoc["value"].as<JsonArray>();
      // log the group
      group3.clear();
      for (JsonVariant led : group) {
        if (led.is<int>()) {
          group3.push_back(led.as<int>());
        }
      }
    } else if (type == "Group4") {
      group = jsonDoc["value"].as<JsonArray>();
      // log the group
      group4.clear();
      for (JsonVariant led : group) {
        if (led.is<int>()) {
          group4.push_back(led.as<int>());
        }
      }
    } else if (type == "Group5") {
      group = jsonDoc["value"].as<JsonArray>();
      // log the group
      group5.clear();
      for (JsonVariant led : group) {
        if (led.is<int>()) {
          group5.push_back(led.as<int>());
        }
      }

    } else if (type == "Start Range") {
      startRange = value.toInt();
    } else if (type == "End Range") {
      endRange = value.toInt();
    } else if (type == "Brightness Audio") {
      if (value == "true") {
        setBrightness = true;
      } else {
        setBrightness = false;
      }
    } else if (type == "Palette") {
      setPalette(value.toInt());
    } else if (type == "Color 1") {
      color1 = hexToCRGB(value);
    } else if (type == "Color 2") {
      color2 = hexToCRGB(value);
    } else if (type == "Color 3") {
      color3 = hexToCRGB(value);
    } else if (type == "Color 4") {
      color4 = hexToCRGB(value);
    } else if (type == "FPS") {
      fps = value.toInt();
    } else if (type == "Fade Amount") {
      fadeAmount = value.toInt();
    } else if (type == "Brightness") {
      brightness = value.toInt();
    } else if (type == "FPS Variability") {
      fpsVariability = 66 - value.toInt();
    } else if (type == "Twinkle Speed") {
      twinkleSpeed = value.toInt();
    } else if (type == "Twinkle Density") {
      twinkleDensity = value.toInt();
    } else if (type == "Cool Like") {
      if (value == "true") {
        coolLikeIncandescentEn = 1;
      } else {
        coolLikeIncandescentEn = 0;
      }
    } else if (type == "Auto Bg") {
      if (value == "true") {
        autoSelectBackgroundColor = 1;
      } else {
        autoSelectBackgroundColor = 0;
      }
    } else if (type == "Aurora Speed") {
      auroraSpeed = value.toInt();
      if (auroraSpeed < 1) auroraSpeed = 1;
      if (auroraSpeed > 20) auroraSpeed = 20;
    } else if (type == "Aurora Intensity") {
      auroraIntensity = value.toInt();
      if (auroraIntensity < 50) auroraIntensity = 50;
      if (auroraIntensity > 255) auroraIntensity = 255;
    } else if (type == "Aurora Wave Count") {
      auroraWaveCount = value.toInt();
      if (auroraWaveCount < 1) auroraWaveCount = 1;
      if (auroraWaveCount > 10) auroraWaveCount = 10;
    } else if (type == "Aurora Color Range") {
      auroraColorRange = value.toInt();
      if (auroraColorRange < 20) auroraColorRange = 20;
      if (auroraColorRange > 120) auroraColorRange = 120;
    } else if (type == "Ring Speed") {
      ringSpeed = value.toInt();
      if (ringSpeed < 1) ringSpeed = 1;
      if (ringSpeed > 30) ringSpeed = 30;
    } else if (type == "Ring Intensity") {
      ringIntensity = value.toInt();
      if (ringIntensity < 50) ringIntensity = 50;
      if (ringIntensity > 255) ringIntensity = 255;
    } else if (type == "Tree Taper Ratio") {
      treeTaperRatio = value.toFloat() / 100.0;
      if (treeTaperRatio < 0.1) treeTaperRatio = 0.1;
      if (treeTaperRatio > 1.0) treeTaperRatio = 1.0;
      invalidateRingMap();
    } else if (type == "Test Ring Number") {
      testRingNumber = value.toInt();
      if (testRingNumber < 0) testRingNumber = 0;
      if (testRingNumber > 50) testRingNumber = 50;
    } else if (type == "Bottom Ring LEDs") {
      bottomRingLEDs = value.toInt();
      if (bottomRingLEDs < 20) bottomRingLEDs = 20;
      if (bottomRingLEDs > 150) bottomRingLEDs = 150;
      invalidateRingMap();
    } else if (type == "Middle Ring LEDs") {
      middleRingLEDs = value.toInt();
      if (middleRingLEDs < 5) middleRingLEDs = 5;
      if (middleRingLEDs > 50) middleRingLEDs = 50;
      invalidateRingMap();
    } else if (type == "Top Ring LEDs") {
      topRingLEDs = value.toInt();
      if (topRingLEDs < 5) topRingLEDs = 5;
      if (topRingLEDs > 50) topRingLEDs = 50;
      invalidateRingMap();
    } else if (type == "Top Zone Height") {
      topZoneHeight = value.toFloat() / 100.0;
      if (topZoneHeight < 0.1) topZoneHeight = 0.1;
      if (topZoneHeight > 0.5) topZoneHeight = 0.5;
      invalidateRingMap();
    } else if (type == "Tree Height") {
      treeHeight = value.toFloat();
      if (treeHeight < 24) treeHeight = 24;
      if (treeHeight > 120) treeHeight = 120;
      invalidateRingMap();
    } else if (type == "Breathing Rate") {
      breathingRate = value.toInt();
      if (breathingRate < 1) breathingRate = 1;
      if (breathingRate > 100) breathingRate = 100;
    } else if (type == "Breathing Variability") {
      breathingVariability = value.toInt();
      if (breathingVariability < 0) breathingVariability = 0;
      if (breathingVariability > 50) breathingVariability = 50;
    } else if (type == "Breath Hold Time") {
      breathHoldTime = value.toInt();
      if (breathHoldTime < 0) breathHoldTime = 0;
      if (breathHoldTime > 100) breathHoldTime = 100;
    } else if (type == "Audio Min Amplitude") {
      audioMinAmplitude = value.toFloat();
      if (audioMinAmplitude < 1.0) audioMinAmplitude = 1.0;
      if (audioMinAmplitude > 100.0) audioMinAmplitude = 100.0;
    } else if (type == "Audio Max Amplitude") {
      audioMaxAmplitude = value.toFloat();
      if (audioMaxAmplitude < 100.0) audioMaxAmplitude = 100.0;
      if (audioMaxAmplitude > 5000.0) audioMaxAmplitude = 5000.0;
    } else if (type == "Audio Smoothing") {
      audioSmoothing = value.toFloat() / 100.0;
      if (audioSmoothing < 0.0) audioSmoothing = 0.0;
      if (audioSmoothing > 1.0) audioSmoothing = 1.0;
    } else if (type == "Audio Color Speed") {
      audioColorSpeed = value.toInt();
      if (audioColorSpeed < 1) audioColorSpeed = 1;
      if (audioColorSpeed > 100) audioColorSpeed = 100;
    } else if (type == "Audio Wave Speed") {
      audioWaveSpeed = value.toInt();
      if (audioWaveSpeed < 1) audioWaveSpeed = 1;
      if (audioWaveSpeed > 100) audioWaveSpeed = 100;
    } else if (type == "Auto Brightness Enabled") {
      if (value == "true") {
        autoBrightnessEnabled = true;
      } else {
        autoBrightnessEnabled = false;
      }
    } else if (type == "Auto Brightness Min Brightness") {
      autoBrightnessMinBrightness = value.toInt();
      if (autoBrightnessMinBrightness < 0) autoBrightnessMinBrightness = 0;
      if (autoBrightnessMinBrightness > 255) autoBrightnessMinBrightness = 255;
      // Ensure min is not greater than max
      if (autoBrightnessMinBrightness > autoBrightnessMaxBrightness) {
        autoBrightnessMinBrightness = autoBrightnessMaxBrightness;
      }
    } else if (type == "Auto Brightness Max Brightness") {
      autoBrightnessMaxBrightness = value.toInt();
      if (autoBrightnessMaxBrightness < 0) autoBrightnessMaxBrightness = 0;
      if (autoBrightnessMaxBrightness > 255) autoBrightnessMaxBrightness = 255;
      // Ensure max is not less than min
      if (autoBrightnessMaxBrightness < autoBrightnessMinBrightness) {
        autoBrightnessMaxBrightness = autoBrightnessMinBrightness;
      }
    } else if (type == "Auto Brightness Min Amplitude") {
      autoBrightnessMinAmplitude = value.toFloat();
      if (autoBrightnessMinAmplitude < 0.0) autoBrightnessMinAmplitude = 0.0;
      if (autoBrightnessMinAmplitude > 10000.0) autoBrightnessMinAmplitude = 10000.0;
      // Ensure min is not greater than max
      if (autoBrightnessMinAmplitude > autoBrightnessMaxAmplitude) {
        autoBrightnessMinAmplitude = autoBrightnessMaxAmplitude;
      }
    } else if (type == "Auto Brightness Max Amplitude") {
      autoBrightnessMaxAmplitude = value.toFloat();
      if (autoBrightnessMaxAmplitude < 0.0) autoBrightnessMaxAmplitude = 0.0;
      if (autoBrightnessMaxAmplitude > 10000.0) autoBrightnessMaxAmplitude = 10000.0;
      // Ensure max is not less than min
      if (autoBrightnessMaxAmplitude < autoBrightnessMinAmplitude) {
        autoBrightnessMaxAmplitude = autoBrightnessMinAmplitude;
      }
    }

    // Optionally, send a response back to the client
    String response = "{\"status\":\"OK\"}";
    client->text(response);
  }
}

void setupWebSockets() {
  // Initialize the filesystem
  if (!LittleFS.begin()) {
    Serial.println("An error occurred while mounting LittleFS");
    return;
  }
  
  // WebSocket setup
  ws_logs.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client,
                     AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.println("Log WebSocket client connected");
    }
  });
  server.addHandler(&ws_logs);

  // Serve upload.html specifically at "/upload"
  server.on("/upload", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/upload.html", "text/html");
  });

  // Serve upload.html specifically at "/upload"
  server.on("/chart", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/chart.html", "text/html");
  });

  server.on(
      "/file-upload", HTTP_POST,
      [](AsyncWebServerRequest *request) {
        Serial.println("Upload handler triggered");
        request->send(200, "text/plain", "File uploaded successfully!");
      },
      [](AsyncWebServerRequest *request, String filename, size_t index,
         uint8_t *data, size_t len, bool final) {
        if (!index) {
          Serial.printf("Starting upload: %s\n", filename.c_str());
          File file = LittleFS.open("/" + filename, "w");
          if (!file) {
            Serial.println("Failed to open file for writing");
            request->send(500, "text/plain", "Failed to open file for writing");
            return;
          }
          file.close();
        }
        File file = LittleFS.open("/" + filename, "a");
        if (file) {
          file.write(data, len);
          file.close();
        } else {
          Serial.println("Failed to open file for appending");
          request->send(500, "text/plain", "Failed to open file for appending");
          return;
        }
        if (final) {
          Serial.printf("Upload complete: %s (%u bytes)\n", filename.c_str(),
                        index + len);
        }
      });

  server.on("/list", HTTP_GET, [](AsyncWebServerRequest *request) {
    String fileList = "Files in LittleFS:\n";
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while (file) {
      fileList +=
          String(file.name()) + " (" + String(file.size()) + " bytes)\n";
      file = root.openNextFile();
    }
    request->send(200, "text/plain", fileList);
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    Serial.printf("Unhandled request to: %s\n", request->url().c_str());
    request->send(404, "text/plain", "Not Found");
  });

  // Add WebSocket event handler
  ws_control.onEvent(onWebSocketControlEvent);
  server.addHandler(&ws_control);

  ws_chart.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.println("Chart WebSocket client connected");
    }
  });
  server.addHandler(&ws_chart);

  // Serve the HTML file
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  // Serve the logs page
  server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/logs.html", "text/html");  // Serve logs page
  });

  // Start the server
  server.begin();
}