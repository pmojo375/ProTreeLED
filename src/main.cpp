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

// FFT variables
#define SAMPLES 256
#define SAMPLE_RATE 1600
#define FFT_SCALE 1024
double micBuffer[SAMPLES];

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
}

void loop() {
  // Get the amplitude of the audio signal
  getAmplitude();

  if (mode == 5) {
    EVERY_N_SECONDS(SECONDS_PER_PALETTE) {
      chooseNextColorPalette(gTargetPalette);
    }

    EVERY_N_MILLISECONDS(10) {
      nblendPaletteTowardPalette(gCurrentPalette, gTargetPalette, 12);
    }

    drawTwinkles(leds);

    FastLED.show();
  } else if (millis() - timer > 1000 / (random16(fpsVariability) + fps)) {
    // If the time since the last execution is greater than 1 second divided by
    // the frame rate (plus some variability)

    timer = millis();

    if (mode == 0) {
      // This mode gets 3 random leds to be color2, color3, and color4 and then
      // fades the rest of the leds to color1
      FastLED.setBrightness(brightness);
      glitter(color2, color3, color4);
      fadeTowardColor(leds, NUM_LEDS, color1, fadeAmount);
    } else if (mode == 1) {
      colorWaves(inc_gHueState, brightness);
    } else if (mode == 2) {
      twinklingStars(color1, brightness);
    } else if (mode == 3) {
      candyCane(color1, color2, brightness);
    } else if (mode == 4) {
      risingSparklesEffect();
    } else if (mode == 6) {
      random16_add_entropy(random());
      Fire2012WithPalette();  // run simulation frame, using palette colors
    } else if (mode == 7) {
      // set range of lights to white and on

      // set all lights to off
      fill_solid(leds, NUM_LEDS, CRGB::Black);

      for (int i = startRange; i < endRange; i++) {
        leds[i] = CRGB::White;
      }
    } else if (mode == 8) {
      // set all lights to off
      fill_solid(leds, NUM_LEDS, CRGB::Black);

      for (int i = 0; i < group1.size(); i++) {
        if (group1[i] >= 0 && group1[i] < NUM_LEDS) {
          leds[group1[i]] = CRGB::White;
        } else {
          broadcastLog("Invalid LED number");
        }
      }
    }

    // send the 'leds' array out to the actual LED strip
    FastLED.show();
  }

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