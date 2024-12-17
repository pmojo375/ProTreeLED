#include <Arduino.h>
#include <ArduinoFFT.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include <LittleFS.h>
#include <OTEUpdater.h>
#include <WiFi.h>
#include <driver/i2s.h>
#include <ledFunctions.h>
#include <time.h>
#include <webSockets.h>

#define MAX_BRIGHTNESS 255

// Time configuration
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000;    // Example: -5 hours for EST
const int daylightOffset_sec = 3600;  // 1 hour for DST

// I2S Pins
#define I2S_WS_PIN 15   // Word Select
#define I2S_SCK_PIN 14  // Clock
#define I2S_SD_PIN 34   // Serial Data

// FFT Settings
#define SAMPLES 256       // Must be a power of 2
#define SAMPLE_RATE 1600  // Sample rate in Hz
double vReal[SAMPLES];    // Real part of FFT
double vImag[SAMPLES];    // Imaginary part of FFT

// ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES,
// SAMPLE_RATE);

// I2S Configuration
i2s_config_t i2s_config = {.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
                           .sample_rate = SAMPLE_RATE,
                           .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
                           .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
                           .communication_format = I2S_COMM_FORMAT_STAND_I2S,
                           .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
                           .dma_buf_count = 8,
                           .dma_buf_len = 1024};

i2s_pin_config_t pin_config = {.bck_io_num = I2S_SCK_PIN,
                               .ws_io_num = I2S_WS_PIN,
                               .data_out_num = I2S_PIN_NO_CHANGE,
                               .data_in_num = I2S_SD_PIN};

// Frequency band limits (in Hz)
const int lowBandStart = 20;
const int lowBandEnd = 250;
const int midBandStart = 251;
const int midBandEnd = 2000;
const int highBandStart = 2001;
const int highBandEnd = 8000;

TaskHandle_t fftTaskHandle;

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

void chartMic(long high) {
  // send all 3 bands to the web server to be charted'
  String message = "{\"high\":" + String(high) + "}";
  ws_logs.textAll(message);
  Serial.println(message);
}

// FFT processing task
void fftTask(void *parameter) {
  for (;;) {
    /*
    int32_t audio_buffer[SAMPLES];
    size_t bytes_read = 0;

    // Read audio samples
    i2s_read(I2S_NUM_0, audio_buffer, sizeof(audio_buffer), &bytes_read,
    portMAX_DELAY);

    // Check if we received valid data
    if (bytes_read == 0) {
        broadcastLog("No audio data received!");
    } else {
        broadcastLog("Bytes read: ");
        broadcastLog(String(bytes_read));
    }
    int samples_read = bytes_read / sizeof(int32_t);
    // Calculate amplitude
    long amplitude = 0;
    for (int i = 0; i < samples_read; i++) {
        amplitude += abs(audio_buffer[i]);
    }

    // broadcast log every second with the amplitude and brightness
    broadcastLog("Amplitude: " + String(amplitude) + " Brightness: " +
    String(brightness));

    if(setBrightness) {
      brightness = map(amplitude, 0, 2500, 50, MAX_BRIGHTNESS);
      brightness = constrain(brightness, 0, MAX_BRIGHTNESS);
      FastLED.setBrightness(brightness);
    }
    */
    /*
    int16_t audio_buffer[SAMPLES];
    size_t bytes_read;

    // Read audio samples from I2S
    i2s_read(I2S_NUM_0, audio_buffer, sizeof(audio_buffer), &bytes_read,
    portMAX_DELAY);

    // Load audio data into FFT arrays
    for (int i = 0; i < SAMPLES; i++) {
        vReal[i] = (double)(audio_buffer[i] / 32768.0);
        vImag[i] = 0.0;
    }

    // Perform FFT
    FFT.windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.complexToMagnitude(vReal, vImag, SAMPLES);

    // Analyze frequency bands
    int low = 0, mid = 0, high = 0;
    for (int i = 1; i < SAMPLES / 2; i++) {
        double frequency = (i * SAMPLE_RATE) / SAMPLES;
        if (frequency < 200) {
            low += (int)vReal[i];
        } else if (frequency < 1000) {
            mid += (int)vReal[i];
        } else {
            high += (int)vReal[i];
        }
    }

    // Map values to LED brightness levels
    int low_brightness = map(low, 0, 10000, 0, 255);
    int mid_brightness = map(mid, 0, 10000, 0, 255);
    int high_brightness = map(high, 0, 10000, 0, 255);

    //chartMic(high_brightness, mid_brightness, low_brightness);
    chartMic(high, mid, low);
    delay(10);  // Adjust delay for the desired FFT update rate
    */
  }
}

FASTLED_USING_NAMESPACE

const char *ssid = "Mojo";          // Replace with your Wi-Fi SSID
const char *password = "Hank0402";  // Replace with your Wi-Fi password

// Timers
unsigned long timer;

// Default Colors
CRGB color1 = CRGB::DarkGreen;
CRGB color2 = CRGB::Red;
CRGB color3 = CRGB::Blue;
CRGB color4 = CRGB::WhiteSmoke;

// Default Parameters
int fpsVariability = 50;
int fps = 10;
bool inc_gHueState = false;
uint8_t fadeAmount = 16;
int mode = 0;
bool setBrightness = false;
uint8_t brightness = 255;

CRGBPalette16 gCurrentPalette;
CRGBPalette16 gTargetPalette;

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

  configWS();

  chooseNextColorPalette(gTargetPalette);

  setupOTA("esp32-ota");

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_32BIT,
              I2S_CHANNEL_MONO);

  timer = millis();

  // Start FFT task on core 1
  // xTaskCreatePinnedToCore(fftTask, "FFT Task", 4096, NULL, 1, &fftTaskHandle,
  //                        1);

  // Initialize time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

long amplitude = 0;
long maxAmplitude = 0;
long minAmplitude = 0;
long avgAmplitude = 0;

void loop() {
  int16_t audio_buffer[SAMPLES];
  size_t bytes_read = 0;
  amplitude = 0;
  // Read audio samples
  i2s_read(I2S_NUM_0, audio_buffer, sizeof(audio_buffer), &bytes_read,
           portMAX_DELAY);

  // Check if we received valid data
  if (bytes_read == 0) {
    broadcastLog("No audio data received!");
  }

  int samples_read = bytes_read / sizeof(int16_t);

  // Calculate RMS amplitude
  float rms = 0;
  for (int i = 0; i < samples_read; i++) {
    rms += audio_buffer[i] * audio_buffer[i];  // Square each sample
  }
  rms = sqrt(rms / samples_read);  // Calculate RMS

  // broadcast log every second with the amplitude and brightness
  // chartMic(amplitude);

  // log max min and ave
  if (rms > maxAmplitude) {
    maxAmplitude = rms;
  }
  if (rms < minAmplitude) {
    minAmplitude = rms;
  }

  broadcastLog("Amplitude: " + String(rms) + " Max: " + String(maxAmplitude) +
               " Min: " + String(minAmplitude));

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