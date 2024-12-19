#include <webSockets.h>

// Create an AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws_logs("/ws_logs");        // WebSocket for logs
AsyncWebSocket ws_chart("/ws_chart");      // WebSocket for mic chart
AsyncWebSocket ws_control("/ws_control");  // WebSocket for your other page

// Initialize an empty JSON document
JsonDocument doc2;

// Create an empty JsonArray
JsonArray group = doc2.add<JsonArray>();

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
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("WebSocket client disconnected");
  } else if (type == WS_EVT_DATA) {
    // Handle incoming data
    JsonDocument jsonDoc;            // Adjust size according to your needs
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
    } else if (type == "Group") {
      group = jsonDoc["value"].as<JsonArray>();

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