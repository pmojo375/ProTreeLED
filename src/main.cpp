#include <Arduino.h>
#include <ArduinoFFT.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#include <OTEUpdater.h>
#include <WiFi.h>
#include <ledFunctions.h>
#include <mic.h>
#include <time.h>
#include <webSockets.h>
// include rtos
#include <freertos/FreeRTOS.h>

#define MAX_BRIGHTNESS 255

// Time configuration
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000;    // Example: -5 hours for EST
const int daylightOffset_sec = 3600;  // 1 hour for DS

TaskHandle_t fftTaskHandle;
TaskHandle_t ledTaskHandle;

// Forward declaration
void ledControlTask(void *parameter);

void chartMic(long high) {
  // send all 3 bands to the web server to be charted'
  String message = "{\"high\":" + String(high) + "}";
  ws_logs.textAll(message);
  Serial.println(message);
}

FASTLED_USING_NAMESPACE

const char *ssid = "Mojo";          // Replace with your Wi-Fi SSID
const char *password = "Hank0402";  // Replace with your Wi-Fi password

// Timers
unsigned long timer;
TickType_t lastWakeTime;

// FFT variables
#define SAMPLES 256
#define SAMPLE_RATE 1600
#define FFT_SCALE 1024
double micBuffer[SAMPLES];

// LED Control Task running on Core 0
void ledControlTask(void *parameter) {
  unsigned long taskTimer = 0;
  
  for (;;) {
    // Calculate frame delay based on FPS
    unsigned long frameDelay = 1000 / (random16(fpsVariability) + fps);
    unsigned long currentTime = millis();
    
    if (mode == 5) {
      // Calculate brightness (with auto brightness if enabled)
      uint8_t effectiveBrightness = calculateAutoBrightness(brightness);
      FastLED.setBrightness(effectiveBrightness);
      
      EVERY_N_SECONDS(SECONDS_PER_PALETTE) {
        chooseNextColorPalette(gTargetPalette);
      }

      EVERY_N_MILLISECONDS(10) {
        nblendPaletteTowardPalette(gCurrentPalette, gTargetPalette, 12);
      }

      drawTwinkles(leds);
      FastLED.show();
      vTaskDelay(pdMS_TO_TICKS(10));
    } else {
      // Check if it's time to update based on FPS
      if (currentTime - taskTimer >= frameDelay) {
        taskTimer = currentTime;
        
        // Calculate brightness (with auto brightness if enabled)
        uint8_t effectiveBrightness = calculateAutoBrightness(brightness);
        
        if (mode == 0) {
          // This mode gets 3 random leds to be color2, color3, and color4 and then
          // fades the rest of the leds to color1
          FastLED.setBrightness(effectiveBrightness);
          glitter(color2, color3, color4);
          fadeTowardColor(leds, NUM_LEDS, color1, fadeAmount);
        } else if (mode == 1) {
          FastLED.setBrightness(effectiveBrightness);
          colorWaves(inc_gHueState, effectiveBrightness);
        } else if (mode == 2) {
          FastLED.setBrightness(effectiveBrightness);
          twinklingStars(color1, effectiveBrightness);
        } else if (mode == 3) {
          FastLED.setBrightness(effectiveBrightness);
          candyCane(color1, color2, effectiveBrightness);
        } else if (mode == 4) {
          FastLED.setBrightness(effectiveBrightness);
          risingSparklesEffect(effectiveBrightness);
        } else if (mode == 6) {
          FastLED.setBrightness(effectiveBrightness);
          random16_add_entropy(random());
          Fire2012WithPalette();  // run simulation frame, using palette colors
        } else if (mode == 9) {
          FastLED.setBrightness(effectiveBrightness);
          auroraBorealis(effectiveBrightness);
        } else if (mode == 10) {
          FastLED.setBrightness(effectiveBrightness);
          spiralRings(effectiveBrightness);
        } else if (mode == 11) {
          FastLED.setBrightness(effectiveBrightness);
          expandingRings(effectiveBrightness);
        } else if (mode == 12) {
          FastLED.setBrightness(effectiveBrightness);
          chasingRings(effectiveBrightness);
        } else if (mode == 13) {
          FastLED.setBrightness(effectiveBrightness);
          gradientRings(effectiveBrightness);
        } else if (mode == 14) {
          FastLED.setBrightness(effectiveBrightness);
          twinklingRings(effectiveBrightness);
        } else if (mode == 15) {
          FastLED.setBrightness(effectiveBrightness);
          waveRings(effectiveBrightness);
        } else if (mode == 16) {
          FastLED.setBrightness(effectiveBrightness);
          singleRingTest(effectiveBrightness);
        } else if (mode == 17) {
          FastLED.setBrightness(effectiveBrightness);
          breathingEffect(effectiveBrightness);
        } else if (mode == 18) {
          // Audio amplitude visualization
          audioAmplitudeEffect(effectiveBrightness);
        } else if (mode == 19) {
          // FFT equalizer visualization
          fftEqualizerEffect(effectiveBrightness);
        } else if (mode == 20) {
          // Audio color spectrum visualization - amplitude-based color shifting
          audioColorSpectrumEffect(effectiveBrightness);
        } else if (mode == 7) {
          // set range of lights to white and on
          FastLED.setBrightness(effectiveBrightness);

          // set all lights to off
          fill_solid(leds, NUM_LEDS, CRGB::Black);

          // Bounds check to prevent array overflow
          int safeStart = (startRange < 0) ? 0 : (startRange >= NUM_LEDS ? NUM_LEDS - 1 : startRange);
          int safeEnd = (endRange < 0) ? 0 : (endRange > NUM_LEDS ? NUM_LEDS : endRange);
          
          for (int i = safeStart; i < safeEnd; i++) {
            leds[i] = CRGB::White;
          }
        } else if (mode == 8) {
          // set all lights to off
          FastLED.setBrightness(effectiveBrightness);
          fill_solid(leds, NUM_LEDS, CRGB::Black);

          for (int i = 0; i < group1.size(); i++) {
            if (group1[i] >= 0 && group1[i] < NUM_LEDS) {
              leds[group1[i]] = CRGB::White;
            } else {
              broadcastLog("Invalid LED number");
            }
          }
          
          for (int i = 0; i < group2.size(); i++) {
            if (group2[i] >= 0 && group2[i] < NUM_LEDS) {
              leds[group2[i]] = CRGB::Red;
            } else {
              broadcastLog("Invalid LED number");
            }
          }

          for (int i = 0; i < group3.size(); i++) {
            if (group3[i] >= 0 && group3[i] < NUM_LEDS) {
              leds[group3[i]] = CRGB::Blue;
            } else {
              broadcastLog("Invalid LED number");
            }
          }

          for (int i = 0; i < group4.size(); i++) {
            if (group4[i] >= 0 && group4[i] < NUM_LEDS) {
              leds[group4[i]] = CRGB::Green;
            } else {
              broadcastLog("Invalid LED number");
            }
          }

          for (int i = 0; i < group5.size(); i++) {
            if (group5[i] >= 0 && group5[i] < NUM_LEDS) {
              leds[group5[i]] = CRGB::Yellow;
            } else {
              broadcastLog("Invalid LED number");
            }
          }
        }
        
        // send the 'leds' array out to the actual LED strip
        FastLED.show();
      }
      
      // Small delay to prevent task from hogging CPU
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // Delay the start of the program to allow the serial monitor to connect
  delay(1000);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS)
      .setCorrection(TypicalLEDStrip);

  // Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi");
  setupWiFi(ssid, password);

  setupWebSockets();

  chooseNextColorPalette(gTargetPalette);

  setupOTA("esp32-ota");

  setupI2S();

  timer = millis();

  // Initialize time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Create LED control task pinned to Core 0
  xTaskCreatePinnedToCore(
    ledControlTask,        // Task function
    "LEDControlTask",      // Task name
    10000,                 // Stack size (bytes)
    NULL,                  // Parameter to pass
    1,                     // Task priority (0-25, higher = higher priority)
    &ledTaskHandle,        // Task handle
    0                      // Core ID (0 = Core 0)
  );
  
  Serial.println("LED Control Task created on Core 0");
}

void loop() {
  // Get the amplitude of the audio signal
  getAmplitude();

  // LED control is now handled by the FreeRTOS task on Core 0
  // Main loop handles other tasks like OTA, WebSockets, etc.

  handleOTA();

  ws_control.cleanupClients();
  ws_logs.cleanupClients();
  ws_chart.cleanupClients();

  // Get current time
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  // Extract hour and adjust brightness
  int currentHour = timeinfo.tm_hour;

  delay(20);
}

// Function to calculate brightness based on hour
int calculateBrightness(int hour) {
  if (hour >= 6 && hour < 18) {          // Daytime
    return MAX_BRIGHTNESS;               // Full brightness
  } else if (hour >= 18 && hour < 21) {  // Evening
    return MAX_BRIGHTNESS / 2;           // Medium brightness
  } else {                               // Night
    return MAX_BRIGHTNESS / 10;          // Dimmed brightness
  }
}