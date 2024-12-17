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

#define MAX_BRIGHTNESS 255

// Time configuration
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000;    // Example: -5 hours for EST
const int daylightOffset_sec = 3600;  // 1 hour for DST

// Create an AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws_logs("/ws_logs");        // WebSocket for logs
AsyncWebSocket ws_chart("/ws_chart");      // WebSocket for mic chart
AsyncWebSocket ws_control("/ws_control");  // WebSocket for your other page

// I2S Pins
#define I2S_WS_PIN 15   // Word Select
#define I2S_SCK_PIN 14  // Clock
#define I2S_SD_PIN 34   // Serial Data

// FFT Settings
#define SAMPLES 256       // Must be a power of 2
#define SAMPLE_RATE 1600  // Sample rate in Hz
double vReal[SAMPLES];    // Real part of FFT
double vImag[SAMPLES];    // Imaginary part of FFT

uint8_t brightness = 255;
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

bool setBrightness = false;

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

// setup millis timer
unsigned long timer;
unsigned long micMillis = 0;
unsigned long micInterval = 0;

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
int mode = 0;

String messageFromClient = "";

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
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("WebSocket client disconnected");
  } else if (type == WS_EVT_DATA) {
    // Handle incoming data
    JsonDocument doc;            // Adjust size according to your needs
    deserializeJson(doc, data);  // Parse the JSON data

    String type = doc["type"];    // Get the type of message
    String value = doc["value"];  // Get the value

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
  chooseNextColorPalette(gTargetPalette);

  setupOTA("esp32-ota");

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_32BIT,
              I2S_CHANNEL_MONO);

  timer = millis();
  micInterval = millis();

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