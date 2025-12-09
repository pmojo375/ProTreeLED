#ifndef ledFunctions_h
#define ledFunctions_h

#include <FastLED.h>

#define NUM_LEDS 400
#define DATA_PIN 5
#define LED_TYPE WS2811
#define COLOR_ORDER RGB
#define SECONDS_PER_PALETTE  30
#define COOLING  20
#define SPARKING 120


//extern CRGB leds[NUM_LEDS];
extern CRGBArray<NUM_LEDS> leds;
extern CRGBPalette16 gCurrentPalette;
extern CRGBPalette16 gTargetPalette;

extern int twinkleSpeed;
extern int twinkleDensity;
extern int secondsPerPalette;
extern int autoSelectBackgroundColor;
extern int coolLikeIncandescentEn;
extern int startRange;
extern int endRange;

// Aurora Borealis Parameters
extern int auroraSpeed;
extern int auroraIntensity;
extern int auroraWaveCount;
extern int auroraColorRange;

// Ring-based Pattern Parameters
extern int ringSpeed;
extern int ringIntensity;
extern float treeHeight; // Tree height in inches
extern float treeTaperRatio; // Ratio of top diameter to bottom diameter (0.0 to 1.0)
extern int testRingNumber; // Ring number to display for testing
extern int bottomRingLEDs; // Target LEDs per ring in bottom zone (50-100)
extern int middleRingLEDs; // Target LEDs per ring in middle zone (~20)
extern int topRingLEDs; // Target LEDs per ring in top zone (~15)
extern float topZoneHeight; // Height ratio for top zone (0.0 to 1.0, e.g. 0.25 = top 25%)

// Breathing Pattern Parameters
extern int breathingRate; // Speed of breathing cycle (1-100)
extern int breathingVariability; // Random variation in timing (0-50)
extern int breathHoldTime; // Time to hold at peak brightness (0-100)

// Audio Visualization Parameters
extern float audioMinAmplitude; // Minimum amplitude threshold for audio modes
extern float audioMaxAmplitude; // Maximum amplitude threshold for audio modes
extern float audioSmoothing; // Smoothing factor (0.0-1.0, higher = more smoothing)
extern int audioColorSpeed; // Color shift speed for mode 20 (1-100)
extern int audioWaveSpeed; // Wave speed multiplier for mode 20 (1-100)

// Auto Brightness Parameters
extern bool autoBrightnessEnabled; // Enable/disable auto brightness based on amplitude
extern uint8_t autoBrightnessMinBrightness; // Minimum brightness value (0-255)
extern uint8_t autoBrightnessMaxBrightness; // Maximum brightness value (0-255)
extern float autoBrightnessMinAmplitude; // Amplitude threshold for minimum brightness
extern float autoBrightnessMaxAmplitude; // Amplitude threshold for maximum brightness

// Default Colors
extern CRGB color1;
extern CRGB color2;
extern CRGB color3;
extern CRGB color4;

// Default Parameters
extern int fpsVariability;
extern int fps;
extern bool inc_gHueState;
extern uint8_t fadeAmount;
extern int mode;
extern bool setBrightness;
extern uint8_t brightness;

void Fire2012WithPalette();

void drawTwinkles( CRGBSet& L);

CRGB computeOneTwinkle( uint32_t ms, uint8_t salt);

uint8_t attackDecayWave8( uint8_t i);

void coolLikeIncandescent( CRGB& c, uint8_t phase);

void chooseNextColorPalette( CRGBPalette16& pal);

void setPalette(int i);

String CRGBToHex(const CRGB& color);

int getPalette();

void colorWaves(bool increment_gHue, uint8_t brightness);

void twinklingStars(CRGB color1, uint8_t brightness);

void candyCane(CRGB color1, CRGB color2, uint8_t brightness);

void risingSparklesEffect(uint8_t brightness);

void auroraBorealis(uint8_t brightness);

// Ring-based pattern functions
int getRingForLED(int ledIndex);
int getLEDsInRing(int ringIndex);
int getFirstLEDInRing(int ringIndex);
int getRingCount();
float getLEDHeight(int ledIndex);
void invalidateRingMap(); // Call this when ring parameters change
void spiralRings(uint8_t brightness);
void expandingRings(uint8_t brightness);
void chasingRings(uint8_t brightness);
void gradientRings(uint8_t brightness);
void twinklingRings(uint8_t brightness);
void waveRings(uint8_t brightness);

void singleRingTest(uint8_t brightness);
void breathingEffect(uint8_t brightness);

void setAll(CRGB color);

void fadeToBlack(int ledNo, byte fadeValue);
void meteorRain(CRGB color, byte meteorSize, byte meteorTrailDecay, bool meteorRandomDecay, int SpeedDelay);

void glitter(CRGB color1, CRGB color2, CRGB color3);

// Helper function that blends one uint8_t toward another by a given amount
void nblendU8TowardU8(uint8_t &cur, const uint8_t target, uint8_t amount);

// Blend one CRGB color toward another CRGB color by a given amount.
// Blending is linear, and done in the RGB color space.
// This function modifies 'cur' in place.
CRGB fadeTowardColor(CRGB &cur, const CRGB &target, uint8_t amount);

// Fade an entire array of CRGBs toward a given background color by a given amount
// This function modifies the pixel array in place.
void fadeTowardColor(CRGB *L, uint16_t N, const CRGB &bgColor, uint8_t fadeAmount);

// Function to convert a single hexadecimal character to an integer
uint8_t hexCharToUint(char hexChar);

// Function to convert a two-character hexadecimal string to an unsigned byte (uint8_t)
uint8_t hexStringToUint8(String hexString);

// Function to convert a hex color string to a CRGB object
CRGB hexToCRGB(String hexColor);

// Audio visualization functions
void audioAmplitudeEffect(uint8_t brightness);
void fftEqualizerEffect(uint8_t brightness);
void audioColorSpectrumEffect(uint8_t brightness);

// Auto brightness function
uint8_t calculateAutoBrightness(uint8_t baseBrightness);

#endif