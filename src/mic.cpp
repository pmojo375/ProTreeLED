#include <mic.h>

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

time_t micTimer = 0;

long getAmplitude() {
  int16_t audio_buffer[SAMPLES];
  size_t bytes_read = 0;
  long amplitude = 0;

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

  if (millis() - micTimer > RMS_INTERVAL) {
    broadcastLog("RMS: " + String(rms));
    micTimer = millis();
  }
  

  return rms;
}

// I2S Configuration
void setupI2S() {
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_32BIT,
              I2S_CHANNEL_MONO);
}
