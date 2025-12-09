#include <mic.h>
#include <webSockets.h>

// I2S Configuration
i2s_config_t i2s_config = {.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
                           .sample_rate = SAMPLE_RATE,
                           .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
                           .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
                           .communication_format = I2S_COMM_FORMAT_STAND_I2S,
                           .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
                           .dma_buf_count = 8,
                           .dma_buf_len = 256};

i2s_pin_config_t pin_config = {.bck_io_num = I2S_SCK_PIN,
                               .ws_io_num = I2S_WS_PIN,
                               .data_out_num = I2S_PIN_NO_CHANGE,
                               .data_in_num = I2S_SD_PIN};

time_t micTimer = 0;

// FFT object
ArduinoFFT<double> FFT = ArduinoFFT<double>();

long getAmplitude() {
  const int samples_to_read = 256;
  uint32_t buffer[samples_to_read];
  size_t bytes_read = 0;

  i2s_read(I2S_NUM_0, buffer, samples_to_read * sizeof(uint32_t), &bytes_read, portMAX_DELAY);

  if (bytes_read == 0) return 0;

  int samples_read = bytes_read / sizeof(uint32_t);
  if (samples_read <= 0) return 0;

  float sum_sq = 0;

  for (int i = 0; i < samples_read; i++) {
      uint32_t raw = buffer[i];

      int32_t sample = raw >> 8;      // extract 24-bit sample

      if (sample & 0x800000)          // sign bit?
          sample |= 0xFF000000;       // extend to full 32-bit signed

      sum_sq += (float)sample * (float)sample;
  }

  float rms = sqrt(sum_sq / samples_read);

  if (millis() - micTimer > RMS_INTERVAL) {
    broadcastLog("RMS: " + String(rms));
    // Send mic data to WebSocket clients for HTML display
    String micMessage = "{\"type\":\"Mic RMS\",\"value\":\"" + String(rms) + "\"}";
    sendMessageToClients(micMessage);
    micTimer = millis();
  }
  

  return rms;
}

// Get FFT data from audio samples
void getFFTData(double *vReal, double *vImag) {
  const int samples_to_read = SAMPLES;
  uint32_t buffer[samples_to_read];
  size_t bytes_read = 0;

  i2s_read(I2S_NUM_0, buffer, samples_to_read * sizeof(uint32_t), &bytes_read, portMAX_DELAY);

  if (bytes_read == 0) {
    // Fill with zeros if no data
    for (int i = 0; i < SAMPLES; i++) {
      vReal[i] = 0;
      vImag[i] = 0;
    }
    return;
  }

  int samples_read = bytes_read / sizeof(uint32_t);
  if (samples_read <= 0) {
    for (int i = 0; i < SAMPLES; i++) {
      vReal[i] = 0;
      vImag[i] = 0;
    }
    return;
  }
  
  // If we didn't get enough samples, pad with zeros
  if (samples_read < SAMPLES) {
    // Fill what we have, then pad
    for (int i = samples_read; i < SAMPLES; i++) {
      buffer[i] = 0;
    }
    samples_read = SAMPLES;
  }

  // Convert samples to double and normalize
  // 24-bit samples are in the upper 24 bits of 32-bit word
  double maxSample = 0;
  for (int i = 0; i < SAMPLES; i++) {
    uint32_t raw = buffer[i];
    int32_t sample = raw >> 8;      // extract 24-bit sample

    if (sample & 0x800000)          // sign bit?
      sample |= 0xFF000000;         // extend to full 32-bit signed

    // Normalize to -1.0 to 1.0 range (24-bit = 2^23 = 8388608)
    vReal[i] = (double)sample / 8388608.0;
    vImag[i] = 0.0;
    
    // Track max for debugging
    if (abs(vReal[i]) > maxSample) maxSample = abs(vReal[i]);
  }

  // Apply windowing before FFT (try lowercase first, template version uses lowercase)
  FFT.windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  
  // Perform FFT
  FFT.compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  
  // Convert to magnitude
  FFT.complexToMagnitude(vReal, vImag, SAMPLES);
  
  // Zero out DC component (bin 0) as it's not useful for visualization
  vReal[0] = 0;
}

// Get frequency bands for equalizer visualization
void getFFTBands(double *bands) {
  double vReal[SAMPLES];
  double vImag[SAMPLES];
  
  getFFTData(vReal, vImag);
  
  // Group FFT bins into bands
  // Only use first half of FFT results (Nyquist frequency)
  // Skip bin 0 (DC component)
  int usableBins = (SAMPLES / 2) - 1;  // Exclude DC and Nyquist
  int binsPerBand = usableBins / FFT_BANDS;
  
  if (binsPerBand < 1) binsPerBand = 1;
  
  for (int band = 0; band < FFT_BANDS; band++) {
    double sum = 0;
    int startBin = 1 + (band * binsPerBand);  // Start from bin 1 (skip DC)
    int endBin = startBin + binsPerBand;
    
    // Clamp endBin to usable range
    if (endBin > SAMPLES / 2) endBin = SAMPLES / 2;
    
    // Sum magnitudes in this band
    int binCount = 0;
    for (int bin = startBin; bin < endBin && bin < SAMPLES / 2; bin++) {
      sum += vReal[bin];
      binCount++;
    }
    
    // Average
    if (binCount > 0) {
      bands[band] = sum / binCount;
    } else {
      bands[band] = 0;
    }
    
    // Apply aggressive scaling for better visualization
    // Scale to make it more visible and sensitive
    bands[band] = bands[band] * 150.0;  // Increased scale factor for more sensitivity
    
    // Apply logarithmic scaling for better dynamic range
    if (bands[band] > 0) {
      bands[band] = log10(bands[band] + 1) * 30.0;  // Increased multiplier
    }
    
    // Add a boost to make smaller values more visible
    bands[band] = bands[band] * 1.5;  // Additional boost factor
  }
}

// I2S Configuration
void setupI2S() {
  i2s_driver_uninstall(I2S_NUM_0);   // Force reset of I2S peripheral
  delay(50);
  esp_err_t err;

  err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  Serial.print("I2S install: ");
  Serial.println(esp_err_to_name(err));
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_32BIT,
              I2S_CHANNEL_MONO);
  
  i2s_zero_dma_buffer(I2S_NUM_0);
}
