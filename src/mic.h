#ifndef mic_h
#define mic_h

#include <driver/i2s.h>
#include <Arduino.h>
#include <math.h>
#include <time.h>
#include <ArduinoFFT.h>

// I2S Pins
#define I2S_WS_PIN 15   // Word Select
#define I2S_SCK_PIN 14  // Clock
#define I2S_SD_PIN 34   // Serial Data

// Samples
#define SAMPLES 256       // Must be a power of 2
#define SAMPLE_RATE 1600  // Sample rate in Hz

// milliseconds
#define RMS_INTERVAL 1000

// FFT bands for equalizer
#define FFT_BANDS 16  // Number of frequency bands for equalizer

extern time_t micTimer;

long getAmplitude();

// FFT functions
void getFFTData(double *vReal, double *vImag);
void getFFTBands(double *bands);

// I2S Configuration
void setupI2S();

#endif
