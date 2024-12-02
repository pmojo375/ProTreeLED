#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <UMS3.h>
#include <LittleFS.h>
#include <ledFunctions.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <OTEUpdater.h>

FASTLED_USING_NAMESPACE

UMS3 ums3;

const char *ssid = "Mojo";         // Replace with your Wi-Fi SSID
const char *password = "Hank0402"; // Replace with your Wi-Fi password

// setup millis timer
unsigned long timer;

// Default Colors
CRGB color1 = CRGB::DarkGreen;
CRGB color2 = CRGB::Red;
CRGB color3 = CRGB::Blue;
CRGB color4 = CRGB::WhiteSmoke;

CRGBPalette16 gCurrentPalette;
CRGBPalette16 gTargetPalette;

// Defaults
int fpsVariability = 50;
int fps = 10;
bool inc_gHueState = false;
uint8_t fadeAmount = 16;
uint8_t brightness = 255;
int mode = 0;

// Create an AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // WebSocket endpoint

String messageFromClient = "";

uint32_t led_cycle_count = 0;

void sendMessageToClients(const String &message) {
  ws.textAll(message);
}

// OTA task function
void otaTask(void *parameter) {
  while (true) {
    handleOTA();
    vTaskDelay(pdMS_TO_TICKS(20)); // Yield to other tasks for 10ms
  }
}

// WebSocket clients task function
void webSocketTask(void *parameter) {
  while (true) {
     ws.cleanupClients();
    vTaskDelay(pdMS_TO_TICKS(50)); // Yield to other tasks for 10ms
  }
}

// Handle WebSocket events
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("WebSocket client connected");
    sendMessageToClients("{\"type\":\"Mode\",\"value\":\"" + String(mode) + "\"}");
    sendMessageToClients("{\"type\":\"Increment gHue\",\"value\":\"" + String(inc_gHueState) + "\"}");
    sendMessageToClients("{\"type\":\"Palette\",\"value\":\"" + String(getPalette()) + "\"}");
    sendMessageToClients("{\"type\":\"Color 1\",\"value\":\"" + CRGBToHex(color1) + "\"}");
    sendMessageToClients("{\"type\":\"Color 2\",\"value\":\"" + CRGBToHex(color2) + "\"}");
    sendMessageToClients("{\"type\":\"Color 3\",\"value\":\"" + CRGBToHex(color3) + "\"}");
    sendMessageToClients("{\"type\":\"Color 4\",\"value\":\"" + CRGBToHex(color4) + "\"}");
    sendMessageToClients("{\"type\":\"FPS\",\"value\":\"" + String(fps) + "\"}");
    sendMessageToClients("{\"type\":\"Fade Amount\",\"value\":\"" + String(fadeAmount) + "\"}");
    sendMessageToClients("{\"type\":\"Brightness\",\"value\":\"" + String(brightness) + "\"}");
    sendMessageToClients("{\"type\":\"FPS Variability\",\"value\":\"" + String(fpsVariability) + "\"}");
    sendMessageToClients("{\"type\":\"Twinkle Speed\",\"value\":\"" + String(twinkleSpeed) + "\"}");
    sendMessageToClients("{\"type\":\"Twinkle Density\",\"value\":\"" + String(twinkleDensity) + "\"}");
    sendMessageToClients("{\"type\":\"Cool Like\",\"value\":\"" + String(coolLikeIncandescentEn) + "\"}");
    sendMessageToClients("{\"type\":\"Auto Bg\",\"value\":\"" + String(autoSelectBackgroundColor) + "\"}");
  }
  else if (type == WS_EVT_DISCONNECT) {
    Serial.println("WebSocket client disconnected");
  }
  else if (type == WS_EVT_DATA) {
    // Handle incoming data
    DynamicJsonDocument doc(1024); // Adjust size according to your needs
    deserializeJson(doc, data);    // Parse the JSON data

    String type = doc["type"];   // Get the type of message
    String value = doc["value"]; // Get the value

    Serial.println("Type: " + type);
    Serial.println("Value: " + value);

    // Act based on the type of message
    if (type == "Mode") {
      mode = value.toInt();
    }
    else if (type == "Increment gHue") {
      if (value == "true") {
        inc_gHueState = true;
      }
      else {
        inc_gHueState = false;
      }
    }
    else if (type == "Palette") {
      setPalette(value.toInt());
    }
    else if (type == "Color 1") {
      color1 = hexToCRGB(value);
    }
    else if (type == "Color 2") {
      color2 = hexToCRGB(value);
    }
    else if (type == "Color 3") {
      color3 = hexToCRGB(value);
    }
    else if (type == "Color 4") {
      color4 = hexToCRGB(value);
    }
    else if (type == "FPS") {
      fps = value.toInt();
    }
    else if (type == "Fade Amount") {
      fadeAmount = value.toInt();
    }
    else if (type == "Brightness") {
      brightness = value.toInt();
    }
    else if (type == "FPS Variability") {
      fpsVariability = 66 - value.toInt();
    }
    else if (type == "Twinkle Speed") {
      twinkleSpeed = value.toInt();
    }
    else if (type == "Twinkle Density") {
      twinkleDensity = value.toInt();
    }
    else if (type == "Cool Like") {
      if (value == "true") {
        coolLikeIncandescentEn = 1;
      }
      else {
        coolLikeIncandescentEn = 0;
      }
    }
    else if (type == "Auto Bg") {
      if (value == "true") {
        autoSelectBackgroundColor = 1;
      }
      else {
        autoSelectBackgroundColor = 0;
      }
    }

    // Optionally, send a response back to the client
    String response = "{\"status\":\"OK\"}";
    client->text(response);
  }
} 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(DATA_PIN, OUTPUT);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // Delay the start of the program to allow the serial monitor to connect
  delay(5000);

  // Initialize all board peripherals, call this first
  ums3.begin();
  ums3.setPixelBrightness(255 / 3);
  ums3.setPixelPower(true);
  ums3.setPixelColor(UMS3::colorWheel(0));

  // Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi");
  setupWiFi(ssid, password);

  // Initialize the filesystem
  if (!LittleFS.begin()) {
    Serial.println("An error occurred while mounting LittleFS");
    return;
  }

  // Serve the HTML file
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html"); });

  // Start the server
  server.begin();

  // Add WebSocket event handler
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  chooseNextColorPalette(gTargetPalette);

  setupOTA("esp32-ota");

	timer = millis();

  // Create OTA task
  xTaskCreatePinnedToCore(
      otaTask,    // Task function
      "OTA Task", // Task name
      4096,       // Stack size in bytes
      NULL,       // Task input parameter
      1,          // Task priority
      NULL,       // Task handle
      0           // Core to pin the task to (0 or 1, or tskNO_AFFINITY for no pinning)
  );  // Create OTA task

  xTaskCreatePinnedToCore(
      webSocketTask,    // Task function
      "Web Socket Task", // Task name
      4096,       // Stack size in bytes
      NULL,       // Task input parameter
      1,          // Task priority
      NULL,       // Task handle
      0           // Core to pin the task to (0 or 1, or tskNO_AFFINITY for no pinning)
  );
}

void loop() {

  if (mode == 5) {
    Serial.println("Mode 1");
		EVERY_N_SECONDS(SECONDS_PER_PALETTE) { 
		  chooseNextColorPalette( gTargetPalette ); 
		}
	
		EVERY_N_MILLISECONDS(10) {
		  nblendPaletteTowardPalette( gCurrentPalette, gTargetPalette, 12);
		}

		drawTwinkles(leds);
	
		FastLED.show();
	} else if (timer - millis() > 1000/ (random16(fpsVariability) + fps) ) {

		if (mode == 0) {
      Serial.println("Mode 0");
			glitter(color2, color3, color4);
			fadeTowardColor(leds, NUM_LEDS, color1, fadeAmount);
		}
		else if (mode == 1) {
      Serial.println("Mode 1");
			colorWaves(inc_gHueState, brightness);
		}
		else if (mode == 2) {
      Serial.println("Mode 2");
			twinklingStars(color1);
		}
		else if (mode == 3) {
      Serial.println("Mode 3");
			candyCane(color1, color2);
		}
		else if (mode == 4) {
      Serial.println("Mode 4");
			risingSparklesEffect();
		} else if (mode == 6) {
      Serial.println("Mode 6");
			
		  random16_add_entropy(random());
		  Fire2012WithPalette(); // run simulation frame, using palette colors
		}

		// send the 'leds' array out to the actual LED strip
		FastLED.show();
	}

	delay(20);
}