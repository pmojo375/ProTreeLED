#include <ledFunctions.h>
#include <mic.h>

CRGBPalette16 currentPalette = RainbowColors_p;
//CRGB leds[NUM_LEDS];
CRGBArray<NUM_LEDS> leds;

CRGB gBackgroundColor = CRGB::Black; 
bool gReverseDirection = false;
CRGBPalette16 gPal = HeatColors_p;

int twinkleSpeed = 4;
int twinkleDensity = 5;
int secondsPerPalette = 30;
int autoSelectBackgroundColor = 0;
int coolLikeIncandescentEn = 1;

// Aurora Borealis Parameters
int auroraSpeed = 3;
int auroraIntensity = 200;
int auroraWaveCount = 3;
int auroraColorRange = 60;

// Ring-based Pattern Parameters
int ringSpeed = 5;
int ringIntensity = 200;
float treeHeight = 84.0; // Tree height in inches (7 feet = 84 inches)
float treeTaperRatio = 0.3; // Top diameter is 30% of bottom diameter (typical tree taper)
int testRingNumber = 20; // Ring number to display for testing (middle ring)
int bottomRingLEDs = 75; // Target LEDs per ring in bottom zone (50-100)
int middleRingLEDs = 20; // Target LEDs per ring in middle zone (~20)
int topRingLEDs = 15; // Target LEDs per ring in top zone (~15)
float topZoneHeight = 0.25; // Height ratio for top zone (0.25 = top 25% of tree)

// Breathing Pattern Parameters
int breathingRate = 30; // Speed of breathing cycle (1-100, higher = faster)
int breathingVariability = 10; // Random variation in timing (0-50)
int breathHoldTime = 20; // Time to hold at peak brightness (0-100, higher = longer hold)

// Audio Visualization Parameters
float audioMinAmplitude = 5.0; // Minimum amplitude threshold
float audioMaxAmplitude = 1500.0; // Maximum amplitude threshold
float audioSmoothing = 0.7; // Smoothing factor (0.0-1.0, higher = more smoothing)
int audioColorSpeed = 30; // Color shift speed for mode 20 (1-100)
int audioWaveSpeed = 50; // Wave speed multiplier for mode 20 (1-100)

// Auto Brightness Parameters
bool autoBrightnessEnabled = false; // Enable/disable auto brightness based on amplitude
uint8_t autoBrightnessMinBrightness = 50; // Minimum brightness value (0-255)
uint8_t autoBrightnessMaxBrightness = 255; // Maximum brightness value (0-255)
float autoBrightnessMinAmplitude = 10.0; // Amplitude threshold for minimum brightness
float autoBrightnessMaxAmplitude = 1000.0; // Amplitude threshold for maximum brightness

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
CRGBPalette16 gCurrentPalette = RainbowColors_p;
CRGBPalette16 gTargetPalette = RainbowColors_p;
int startRange = 0;
int endRange = NUM_LEDS;

void Fire2012WithPalette() {
// Array of temperature readings at each simulation cell
  static uint8_t heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for(int k = NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      uint8_t colorindex = scale8(heat[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
}

void drawTwinkles( CRGBSet& L)
{
  // "PRNG16" is the pseudorandom number generator
  // It MUST be reset to the same starting value each time
  // this function is called, so that the sequence of 'random'
  // numbers that it generates is (paradoxically) stable.
  uint16_t PRNG16 = 11337;
  
  uint32_t clock32 = millis();

  // Set up the background color, "bg".
  // if AUTO_SELECT_BACKGROUND_COLOR == 1, and the first two colors of
  // the current palette are identical, then a deeply faded version of
  // that color is used for the background color
  CRGB bg;
  if( (autoSelectBackgroundColor == 1) &&
      (gCurrentPalette[0] == gCurrentPalette[1] )) {
    bg = gCurrentPalette[0];
    uint8_t bglight = bg.getAverageLight();
    if( bglight > 64) {
      bg.nscale8_video( 16); // very bright, so scale to 1/16th
    } else if( bglight > 16) {
      bg.nscale8_video( 64); // not that bright, so scale to 1/4th
    } else {
      bg.nscale8_video( 86); // dim, scale to 1/3rd.
    }
  } else {
    bg = gBackgroundColor; // just use the explicitly defined background color
  }

  uint8_t backgroundBrightness = bg.getAverageLight();
  
  for( CRGB& pixel: L) {
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    uint16_t myclockoffset16= PRNG16; // use that number as clock offset
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    // use that number as clock speed adjustment factor (in 8ths, from 8/8ths to 23/8ths)
    uint8_t myspeedmultiplierQ5_3 =  ((((PRNG16 & 0xFF)>>4) + (PRNG16 & 0x0F)) & 0x0F) + 0x08;
    uint32_t myclock30 = (uint32_t)((clock32 * myspeedmultiplierQ5_3) >> 3) + myclockoffset16;
    uint8_t  myunique8 = PRNG16 >> 8; // get 'salt' value for this pixel

    // We now have the adjusted 'clock' for this pixel, now we call
    // the function that computes what color the pixel should be based
    // on the "brightness = f( time )" idea.
    CRGB c = computeOneTwinkle( myclock30, myunique8);

    uint8_t cbright = c.getAverageLight();
    int16_t deltabright = cbright - backgroundBrightness;
    if( deltabright >= 32 || (!bg)) {
      // If the new pixel is significantly brighter than the background color, 
      // use the new color.
      pixel = c;
    } else if( deltabright > 0 ) {
      // If the new pixel is just slightly brighter than the background color,
      // mix a blend of the new color and the background color
      pixel = blend( bg, c, deltabright * 8);
    } else { 
      // if the new pixel is not at all brighter than the background color,
      // just use the background color.
      pixel = bg;
    }
  }
}

CRGB computeOneTwinkle( uint32_t ms, uint8_t salt)
{
  uint16_t ticks = ms >> (8-twinkleSpeed);
  uint8_t fastcycle8 = ticks;
  uint16_t slowcycle16 = (ticks >> 8) + salt;
  slowcycle16 += sin8( slowcycle16);
  slowcycle16 =  (slowcycle16 * 2053) + 1384;
  uint8_t slowcycle8 = (slowcycle16 & 0xFF) + (slowcycle16 >> 8);
  
  uint8_t bright = 0;
  if( ((slowcycle8 & 0x0E)/2) < twinkleDensity) {
    bright = attackDecayWave8( fastcycle8);
  }

  uint8_t hue = slowcycle8 - salt;
  CRGB c;
  if( bright > 0) {
    c = ColorFromPalette( gCurrentPalette, hue, bright, NOBLEND);
    if( coolLikeIncandescentEn == 1 ) {
      coolLikeIncandescent( c, fastcycle8);
    }
  } else {
    c = CRGB::Black;
  }
  return c;
}

uint8_t attackDecayWave8( uint8_t i)
{
  if( i < 86) {
    return i * 3;
  } else {
    i -= 86;
    return 255 - (i + (i/2));
  }
}

void coolLikeIncandescent( CRGB& c, uint8_t phase)
{
  if( phase < 128) return;

  uint8_t cooling = (phase - 128) >> 4;
  c.g = qsub8( c.g, cooling);
  c.b = qsub8( c.b, cooling * 2);
}

// A mostly red palette with green accents and white trim.
// "CRGB::Gray" is used as white to keep the brightness more uniform.
const TProgmemRGBPalette16 RedGreenWhite_p FL_PROGMEM =
{  CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 
   CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 
   CRGB::Red, CRGB::Red, CRGB::Gray, CRGB::Gray, 
   CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green };

// A mostly (dark) green palette with red berries.
#define Holly_Green 0x00580c
#define Holly_Red   0xB00402
const TProgmemRGBPalette16 Holly_p FL_PROGMEM =
{  Holly_Green, Holly_Green, Holly_Green, Holly_Green, 
   Holly_Green, Holly_Green, Holly_Green, Holly_Green, 
   Holly_Green, Holly_Green, Holly_Green, Holly_Green, 
   Holly_Green, Holly_Green, Holly_Green, Holly_Red 
};

// A red and white striped palette
// "CRGB::Gray" is used as white to keep the brightness more uniform.
const TProgmemRGBPalette16 RedWhite_p FL_PROGMEM =
{  CRGB::Red,  CRGB::Red,  CRGB::Red,  CRGB::Red, 
   CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray,
   CRGB::Red,  CRGB::Red,  CRGB::Red,  CRGB::Red, 
   CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray };

// A mostly blue palette with white accents.
// "CRGB::Gray" is used as white to keep the brightness more uniform.
const TProgmemRGBPalette16 BlueWhite_p FL_PROGMEM =
{  CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, 
   CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, 
   CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, 
   CRGB::Blue, CRGB::Gray, CRGB::Gray, CRGB::Gray };

// A pure "fairy light" palette with some brightness variations
#define HALFFAIRY ((CRGB::FairyLight & 0xFEFEFE) / 2)
#define QUARTERFAIRY ((CRGB::FairyLight & 0xFCFCFC) / 4)
const TProgmemRGBPalette16 FairyLight_p FL_PROGMEM =
{  CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, 
   HALFFAIRY,        HALFFAIRY,        CRGB::FairyLight, CRGB::FairyLight, 
   QUARTERFAIRY,     QUARTERFAIRY,     CRGB::FairyLight, CRGB::FairyLight, 
   CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight };

// A palette of soft snowflakes with the occasional bright one
const TProgmemRGBPalette16 Snow_p FL_PROGMEM =
{  0x304048, 0x304048, 0x304048, 0x304048,
   0x304048, 0x304048, 0x304048, 0x304048,
   0x304048, 0x304048, 0x304048, 0x304048,
   0x304048, 0x304048, 0x304048, 0xE0F0FF };

// A palette reminiscent of large 'old-school' C9-size tree lights
// in the five classic colors: red, orange, green, blue, and white.
#define C9_Red    0xB80400
#define C9_Orange 0x902C02
#define C9_Green  0x046002
#define C9_Blue   0x070758
#define C9_White  0x606820
const TProgmemRGBPalette16 RetroC9_p FL_PROGMEM =
{  C9_Red,    C9_Orange, C9_Red,    C9_Orange,
   C9_Orange, C9_Red,    C9_Orange, C9_Red,
   C9_Green,  C9_Green,  C9_Green,  C9_Green,
   C9_Blue,   C9_Blue,   C9_Blue,
   C9_White
};

// A cold, icy pale blue palette
#define Ice_Blue1 0x0C1040
#define Ice_Blue2 0x182080
#define Ice_Blue3 0x5080C0
const TProgmemRGBPalette16 Ice_p FL_PROGMEM =
{
  Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
  Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
  Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
  Ice_Blue2, Ice_Blue2, Ice_Blue2, Ice_Blue3
};

// Add or remove palette names from this list to control which color
// palettes are used, and in what order.
const TProgmemRGBPalette16* ActivePaletteList[] = {
  &RetroC9_p,
  &BlueWhite_p,
  &RainbowColors_p,
  &FairyLight_p,
  &RedGreenWhite_p,
  &PartyColors_p,
  &RedWhite_p,
  &Snow_p,
  &Holly_p,
  &Ice_p  
};

// Advance to the next color palette in the list (above).
void chooseNextColorPalette( CRGBPalette16& pal)
{
  const uint8_t numberOfPalettes = sizeof(ActivePaletteList) / sizeof(ActivePaletteList[0]);
  static uint8_t whichPalette = -1; 
  whichPalette = addmod8( whichPalette, 1, numberOfPalettes);

  pal = *(ActivePaletteList[whichPalette]);
}
void setPalette(int i) {
  switch (i) {
    case 0:
      currentPalette = RainbowColors_p;
      break;
    case 1:
      currentPalette = RainbowStripeColors_p;
      break;
    case 2:
      currentPalette = CloudColors_p;
      break;
    case 3:
      currentPalette = LavaColors_p;
      break;
    case 4:
      currentPalette = OceanColors_p;
      break;
    case 5:
      currentPalette = ForestColors_p;
      break;
    case 6:
      currentPalette = PartyColors_p;
      break;
    case 7:
      currentPalette = HeatColors_p;
      break;
    case 8:
      currentPalette = RetroC9_p;
      break;
    case 9:
      currentPalette = BlueWhite_p;
      break;
    case 10:
      currentPalette = FairyLight_p;
      break;
    case 11:
      currentPalette = RedGreenWhite_p;
      break;
    case 12:
      currentPalette = RedWhite_p;
      break;
    case 13:
      currentPalette = Snow_p;
      break;
    case 14:
      currentPalette = Holly_p;
      break;
    case 15:
      currentPalette = Ice_p;
      break;
  }
}

// Function to convert a CRGB value to a hex color string
String CRGBToHex(const CRGB& color) {
    char hexColor[8]; // Buffer to hold the resulting hex string

    sprintf(hexColor, "#%02X%02X%02X", color.r, color.g, color.b);
    return String(hexColor);
}

int getPalette() {
  if (currentPalette == RainbowColors_p) {
    return 0;
  } else if (currentPalette == RainbowStripeColors_p) {
    return 1;
  } else if (currentPalette == CloudColors_p) {
    return 2;
  } else if (currentPalette == LavaColors_p) {
    return 3;
  } else if (currentPalette == OceanColors_p) {
    return 4;
  } else if (currentPalette == ForestColors_p) {
    return 5;
  } else if (currentPalette == PartyColors_p) {
    return 6;
  } else if (currentPalette == HeatColors_p) {
    return 7;
  } else if (currentPalette == RetroC9_p) {
    return 8;
  } else if (currentPalette == BlueWhite_p) {
    return 9;
  } else if (currentPalette == FairyLight_p) {
    return 10;
  } else if (currentPalette == RedGreenWhite_p) {
    return 11;
  } else if (currentPalette == RedWhite_p) {
    return 12;
  } else if (currentPalette == Snow_p) {
    return 13;
  } else if (currentPalette == Holly_p) {
    return 14;
  } else if (currentPalette == Ice_p) {
    return 15;
  } else {
    return 0;
  }
}

void colorWaves(bool increment_gHue, uint8_t brightness) {
  static uint8_t gHue = 0;
  if (increment_gHue) {
    gHue++;
  }
  fill_palette(leds, NUM_LEDS, gHue, 7, currentPalette, brightness, LINEARBLEND);
}

void twinklingStars(CRGB color1, uint8_t brightness) {
  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = random(NUM_LEDS);
  
  // Bounds check to prevent array overflow
  if (pos < 0) pos = 0;
  if (pos >= NUM_LEDS) pos = NUM_LEDS - 1;

  // Scale the color1 brightness
  color1.nscale8_video(brightness);
  leds[pos] += color1; // cool white color
}

void candyCane(CRGB color1, CRGB color2, uint8_t brightness) {
  static uint8_t stripePattern = 0;
  static uint8_t updateCounter = 0;

  // Scale the brightness of the colors
  color1.nscale8_video(brightness);
  color2.nscale8_video(brightness);

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = (i + stripePattern) % 4 < 2 ? color1 : color2;
  }

  // Increment the stripe pattern every 4th call
  if (++updateCounter >= 4) {
    stripePattern++;
    updateCounter = 0;
  }
}

void risingSparklesEffect(uint8_t brightness) {
    const uint8_t sparkleChance = 25; // Chance of new sparkle
    const uint8_t sparkleFade = 1;  // Sparkle fade out speed
    static uint32_t lastFadeTime = 0;

    // Shift everything up one pixel per frame
    for (int i = 0; i < NUM_LEDS - 1; i++) {
        leds[i] = leds[i + 1];
    }

    // Randomly create new sparkle at the bottom
    if (random8() < sparkleChance) {
        // Scale brightness properly
        uint8_t sparkleBrightness = scale8(255, brightness);
        leds[NUM_LEDS - 1] = CHSV(random8(160, 200), 255, sparkleBrightness); // Cool color range
    } else {
        leds[NUM_LEDS - 1] = CRGB::Black;
    }

    // Fade the LEDs every 5ms (fixed timing issue)
    uint32_t now = millis();
    if (now - lastFadeTime >= 5) {
        fadeToBlackBy(leds, NUM_LEDS, sparkleFade);
        lastFadeTime = now;
    }
}

void auroraBorealis(uint8_t brightness) {
    static uint16_t timeOffset = 0;
    
    // Increment time offset based on speed (higher speed = faster movement)
    timeOffset += auroraSpeed;
    if (timeOffset > 65535) timeOffset = 0;
    
    // Base hue for Aurora (green-blue-purple range)
    // Typical Aurora colors: green (~96), cyan (~128), blue (~160), purple (~192)
    uint8_t baseHue = 96; // Start with green
    
    // Create multiple wave layers for depth
    for (int i = 0; i < NUM_LEDS; i++) {
        float ledPosition = (float)i / NUM_LEDS; // 0.0 to 1.0
        
        // Combine multiple sine waves for organic, flowing motion
        float wave1 = sin((ledPosition * PI * 2 * auroraWaveCount) + (timeOffset * 0.001));
        float wave2 = sin((ledPosition * PI * 2 * auroraWaveCount * 1.5) + (timeOffset * 0.0015));
        float wave3 = sin((ledPosition * PI * 2 * auroraWaveCount * 0.7) + (timeOffset * 0.0007));
        
        // Combine waves with different weights for natural variation
        float combinedWave = (wave1 * 0.5) + (wave2 * 0.3) + (wave3 * 0.2);
        
        // Normalize to 0-1 range
        combinedWave = (combinedWave + 1.0) * 0.5;
        
        // Create vertical variation (height-based intensity)
        float verticalIntensity = sin(ledPosition * PI) * 0.5 + 0.5; // Stronger in middle
        
        // Calculate hue variation across the color range
        uint8_t hue = baseHue + (combinedWave * auroraColorRange);
        
        // Calculate brightness based on wave intensity and vertical position
        uint8_t waveBrightness = combinedWave * auroraIntensity * verticalIntensity;
        waveBrightness = scale8(waveBrightness, brightness);
        
        // Create the color with high saturation for vibrant Aurora
        leds[i] = CHSV(hue, 255, waveBrightness);
    }
}

// Ring mapping system - calculates ring structure based on physical tree dimensions
// New algorithm: Bins LEDs into rings based on tunable target LED counts per zone
// Zones: Bottom (wide, many LEDs per ring), Middle (medium), Top (narrow, few LEDs per ring)

// Calculate circumference at a given height (0.0 = bottom, 1.0 = top)
// Uses taper ratio: circumference scales linearly from 1.0 at bottom to taperRatio at top
float getCircumferenceAtHeight(float heightRatio) {
    // heightRatio: 0.0 = bottom, 1.0 = top
    // Circumference scales linearly: bottom = 1.0, top = taperRatio
    return 1.0 - (heightRatio * (1.0 - treeTaperRatio));
}

// Calculate cumulative LED usage up to a given height
// This accounts for wrapping: more wraps at bottom (wide), fewer at top (narrow)
float getCumulativeLEDsAtHeight(float heightRatio) {
    // Integrate circumference from bottom to this height
    // Since circumference is linear, we can use trapezoidal integration
    // At height h, cumulative LEDs = integral from 0 to h of circumference
    
    // Simplified: circumference(h) = 1 - h*(1-taper)
    // Integral = h - h^2*(1-taper)/2
    float h = heightRatio;
    return h - (h * h * (1.0 - treeTaperRatio) / 2.0);
}

// Calculate which zone (bottom, middle, top) an LED belongs to based on height
// Returns: 0 = bottom, 1 = middle, 2 = top
int getZoneForHeight(float height) {
    float heightRatio = height / treeHeight;
    if (heightRatio >= (1.0 - topZoneHeight)) {
        return 2; // Top zone
    } else if (heightRatio < 0.5) {
        return 0; // Bottom zone (first 50% of height)
    } else {
        return 1; // Middle zone
    }
}

// Get target LEDs per ring for a given zone
int getTargetLEDsForZone(int zone) {
    switch(zone) {
        case 0: return bottomRingLEDs; // Bottom
        case 1: return middleRingLEDs;  // Middle
        case 2: return topRingLEDs;     // Top
        default: return middleRingLEDs;
    }
}

// Pre-calculated ring mapping cache (recalculated when parameters change)
static int ringMap[NUM_LEDS] = {-1}; // -1 means not initialized
static bool ringMapValid = false;

// Recalculate ring mapping based on current parameters
// Uses a more efficient approach: groups LEDs by zone and height, then bins into rings
void recalculateRingMap() {
    // Clear the map
    for (int i = 0; i < NUM_LEDS; i++) {
        ringMap[i] = -1;
    }
    
    // First pass: collect all LEDs with their heights and zones
    struct LEDInfo {
        int index;
        float height;
        int zone;
    };
    
    LEDInfo ledInfos[NUM_LEDS];
    
    for (int i = 0; i < NUM_LEDS; i++) {
        float height = getLEDHeight(i);
        int zone = getZoneForHeight(height);
        ledInfos[i] = {i, height, zone};
    }
    
    // Simple insertion sort by zone, then by height (bottom to top within zone)
    for (int i = 1; i < NUM_LEDS; i++) {
        LEDInfo key = ledInfos[i];
        int j = i - 1;
        
        // Move elements that should come after key
        while (j >= 0 && (ledInfos[j].zone > key.zone || 
                         (ledInfos[j].zone == key.zone && ledInfos[j].height > key.height))) {
            ledInfos[j + 1] = ledInfos[j];
            j--;
        }
        ledInfos[j + 1] = key;
    }
    
    // Second pass: bin LEDs into rings within each zone
    int currentRing = 0;
    int currentZone = -1;
    int ledsInCurrentRing = 0;
    int targetLEDs = middleRingLEDs;
    
    for (int i = 0; i < NUM_LEDS; i++) {
        int zone = ledInfos[i].zone;
        
        // If we've moved to a new zone, reset ring counting
        if (zone != currentZone) {
            currentZone = zone;
            targetLEDs = getTargetLEDsForZone(zone);
            ledsInCurrentRing = 0;
            // Don't increment ring when starting first zone
            if (i > 0) {
                currentRing++;
            }
        }
        
        // Assign LED to current ring
        ringMap[ledInfos[i].index] = currentRing;
        ledsInCurrentRing++;
        
        // If we've reached target LEDs for this ring, start a new ring
        if (ledsInCurrentRing >= targetLEDs) {
            currentRing++;
            ledsInCurrentRing = 0;
        }
    }
    
    ringMapValid = true;
}

// Calculate which ring an LED belongs to based on its position along the strip
// Uses new binning algorithm based on target LED counts per zone
int getRingForLED(int ledIndex) {
    if (ledIndex < 0 || ledIndex >= NUM_LEDS) return -1;
    
    // Recalculate map if parameters changed or first time
    if (!ringMapValid) {
        recalculateRingMap();
    }
    
    return ringMap[ledIndex];
}

// Get the number of LEDs in a specific ring
int getLEDsInRing(int ringIndex) {
    int count = 0;
    for (int i = 0; i < NUM_LEDS; i++) {
        if (getRingForLED(i) == ringIndex) {
            count++;
        }
    }
    return count;
}

// Get the first LED index in a ring
int getFirstLEDInRing(int ringIndex) {
    for (int i = 0; i < NUM_LEDS; i++) {
        if (getRingForLED(i) == ringIndex) {
            return i;
        }
    }
    return -1; // Ring not found
}

// Invalidate ring map (call when parameters change)
void invalidateRingMap() {
    ringMapValid = false;
}

// Get total number of rings (calculated from actual ring assignments)
int getRingCount() {
    if (!ringMapValid) {
        recalculateRingMap();
    }
    
    int maxRing = -1;
    for (int i = 0; i < NUM_LEDS; i++) {
        if (ringMap[i] > maxRing) {
            maxRing = ringMap[i];
        }
    }
    return maxRing + 1; // +1 because rings are 0-indexed
}

// Helper function to get LED height (for debugging/testing)
float getLEDHeight(int ledIndex) {
    if (ledIndex < 0) return 0.0;
    if (ledIndex >= NUM_LEDS) return treeHeight;
    
    float ledPosition = (float)ledIndex / (NUM_LEDS - 1);
    float cumulative = ledPosition;
    
    float a = (1.0 - treeTaperRatio) / 2.0;
    float heightRatio;
    
    if (a < 0.001) {
        heightRatio = cumulative;
    } else {
        float discriminant = 1.0 - 4.0 * a * cumulative;
        if (discriminant < 0) discriminant = 0;
        heightRatio = (1.0 - sqrt(discriminant)) / (2.0 * a);
        if (heightRatio < 0) heightRatio = 0;
        if (heightRatio > 1.0) heightRatio = 1.0;
    }
    
    return heightRatio * treeHeight;
}

// Spiral Rings - rotating pattern that spirals up the tree
void spiralRings(uint8_t brightness) {
    static uint16_t rotationOffset = 0;
    rotationOffset += ringSpeed;
    if (rotationOffset > 65535) rotationOffset = 0;
    
    for (int i = 0; i < NUM_LEDS; i++) {
        int ring = getRingForLED(i);
        int maxRings = getRingCount();
        if (ring < 0 || ring >= maxRings) {
            leds[i] = CRGB::Black;
            continue;
        }
        
        int ledsInRing = getLEDsInRing(ring);
        if (ledsInRing == 0) {
            leds[i] = CRGB::Black;
            continue;
        }
        
        int firstLED = getFirstLEDInRing(ring);
        int positionInRing = i - firstLED;
        
        // Create rotating pattern within each ring
        float angle = ((float)positionInRing / ledsInRing) * TWO_PI;
        float rotation = ((float)rotationOffset / 1000.0) + (ring * 0.3); // Different phase per ring
        
        // Calculate brightness based on sine wave
        float wave = sin(angle + rotation) * 0.5 + 0.5;
        uint8_t waveBrightness = wave * ringIntensity;
        waveBrightness = scale8(waveBrightness, brightness);
        
        // Use palette for color variation based on ring position
        uint8_t colorIndex = (ring * 255 / maxRings) + (rotationOffset / 256);
        CRGB color = ColorFromPalette(currentPalette, colorIndex, waveBrightness, LINEARBLEND);
        leds[i] = color;
    }
}

// Expanding Rings - pulses that expand from bottom to top
void expandingRings(uint8_t brightness) {
    static uint16_t pulseOffset = 0;
    pulseOffset += ringSpeed * 2;
    if (pulseOffset > 65535) pulseOffset = 0;
    
    for (int i = 0; i < NUM_LEDS; i++) {
        int ring = getRingForLED(i);
        int maxRings = getRingCount();
        if (ring < 0 || ring >= maxRings) {
            leds[i] = CRGB::Black;
            continue;
        }
        
        // Create expanding pulse effect
        float ringPosition = (float)ring / maxRings; // 0.0 to 1.0
        float pulsePhase = ((float)pulseOffset / 1000.0) - (ringPosition * 3.0);
        float pulse = sin(pulsePhase) * 0.5 + 0.5;
        
        uint8_t pulseBrightness = pulse * ringIntensity;
        pulseBrightness = scale8(pulseBrightness, brightness);
        
        // Use palette for color gradient from bottom to top
        uint8_t colorIndex = ring * 255 / maxRings;
        CRGB color = ColorFromPalette(currentPalette, colorIndex, pulseBrightness, LINEARBLEND);
        leds[i] = color;
    }
}

// Chasing Rings - rings light up sequentially
void chasingRings(uint8_t brightness) {
    static uint16_t chaseOffset = 0;
    chaseOffset += ringSpeed * 3;
    if (chaseOffset > 65535) chaseOffset = 0;
    
    for (int i = 0; i < NUM_LEDS; i++) {
        int ring = getRingForLED(i);
        int maxRings = getRingCount();
        if (ring < 0 || ring >= maxRings) {
            leds[i] = CRGB::Black;
            continue;
        }
        
        // Calculate which rings are lit (chasing effect)
        float chasePhase = (float)chaseOffset / 1000.0;
        float ringPhase = (float)ring / maxRings;
        float distance = fabs(ringPhase - fmod(chasePhase, 1.0));
        if (distance > 0.5) distance = 1.0 - distance;
        
        // Create smooth falloff
        float intensity = 1.0 - (distance * 2.0);
        if (intensity < 0) intensity = 0;
        intensity = intensity * intensity; // Square for smoother falloff
        
        uint8_t ringBrightness = intensity * ringIntensity;
        ringBrightness = scale8(ringBrightness, brightness);
        
        // Use color1 for the chase
        CRGB color = color1;
        color.nscale8_video(ringBrightness);
        leds[i] = color;
    }
}

// Gradient Rings - smooth color gradient from bottom to top
void gradientRings(uint8_t brightness) {
    static uint16_t gradientOffset = 0;
    gradientOffset += ringSpeed;
    if (gradientOffset > 65535) gradientOffset = 0;
    
    for (int i = 0; i < NUM_LEDS; i++) {
        int ring = getRingForLED(i);
        int maxRings = getRingCount();
        if (ring < 0 || ring >= maxRings) {
            leds[i] = CRGB::Black;
            continue;
        }
        
        int ledsInRing = getLEDsInRing(ring);
        if (ledsInRing == 0) {
            leds[i] = CRGB::Black;
            continue;
        }
        
        int firstLED = getFirstLEDInRing(ring);
        int positionInRing = i - firstLED;
        
        // Gradient within ring
        float ringGradient = (float)positionInRing / ledsInRing;
        
        // Overall gradient from bottom to top
        float verticalGradient = (float)ring / maxRings;
        
        // Use palette for gradient - perfect match!
        float huePosition = verticalGradient + ((float)gradientOffset / 65535.0);
        uint8_t colorIndex = (huePosition * 255) + (ringGradient * 30);
        
        uint8_t ringBrightness = ringIntensity;
        ringBrightness = scale8(ringBrightness, brightness);
        
        CRGB color = ColorFromPalette(currentPalette, colorIndex, ringBrightness, LINEARBLEND);
        leds[i] = color;
    }
}

// Twinkling Rings - random rings twinkle
void twinklingRings(uint8_t brightness) {
    static uint32_t lastUpdate = 0;
    static bool ringStates[100] = {false}; // Support up to 100 rings
    const int maxStoredRings = sizeof(ringStates) / sizeof(ringStates[0]);
    
    // Update ring states periodically
    int updateInterval = (ringSpeed > 0) ? (1000 / ringSpeed) : 1000;
    if (millis() - lastUpdate > updateInterval) {
        lastUpdate = millis();
        
        int maxRings = getRingCount();
        int ringsToProcess = min(maxRings, maxStoredRings);
        // Randomly toggle rings
        for (int r = 0; r < ringsToProcess; r++) {
            if (random8() < 15) { // 15% chance to toggle
                ringStates[r] = !ringStates[r];
            }
        }
    }
    
    // Fade all LEDs
    fadeToBlackBy(leds, NUM_LEDS, 20);
    
    // Light up active rings
    int maxRings = getRingCount();
    int ringsToRender = min(maxRings, maxStoredRings);
    for (int i = 0; i < NUM_LEDS; i++) {
        int ring = getRingForLED(i);
        if (ring >= 0 && ring < ringsToRender && ringStates[ring]) {
            uint8_t ringBrightness = ringIntensity;
            ringBrightness = scale8(ringBrightness, brightness);
            
            // Use palette with random variation for twinkling effect
            uint8_t colorIndex = (ring * 255 / maxRings) + random8(30);
            CRGB color = ColorFromPalette(currentPalette, colorIndex, ringBrightness, LINEARBLEND);
            leds[i] = color;
        }
    }
}

// Wave Rings - wave traveling up or down the tree
void waveRings(uint8_t brightness) {
    static uint16_t waveOffset = 0;
    waveOffset += ringSpeed * 2;
    if (waveOffset > 65535) waveOffset = 0;
    
    for (int i = 0; i < NUM_LEDS; i++) {
        int ring = getRingForLED(i);
        int maxRings = getRingCount();
        if (ring < 0 || ring >= maxRings) {
            leds[i] = CRGB::Black;
            continue;
        }
        
        int ledsInRing = getLEDsInRing(ring);
        if (ledsInRing == 0) {
            leds[i] = CRGB::Black;
            continue;
        }
        
        int firstLED = getFirstLEDInRing(ring);
        int positionInRing = i - firstLED;
        
        // Wave traveling through rings
        float ringPosition = (float)ring / maxRings;
        float wavePhase = ((float)waveOffset / 1000.0) - (ringPosition * 4.0);
        float wave = sin(wavePhase) * 0.5 + 0.5;
        
        // Add rotation within ring
        float rotation = ((float)positionInRing / ledsInRing) * TWO_PI;
        float ringWave = sin(rotation + wavePhase) * 0.3 + 0.7;
        
        float combinedWave = wave * ringWave;
        uint8_t waveBrightness = combinedWave * ringIntensity;
        waveBrightness = scale8(waveBrightness, brightness);
        
        // Use palette for color based on ring and wave position
        uint8_t colorIndex = (ring * 255 / maxRings) + (waveOffset / 256);
        CRGB color = ColorFromPalette(currentPalette, colorIndex, waveBrightness, LINEARBLEND);
        leds[i] = color;
    }
}

// Single Ring Test - displays only one ring for testing tree parameters
void singleRingTest(uint8_t brightness) {
    // Clear all LEDs
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    
    int maxRings = getRingCount();
    int ringToShow = testRingNumber;
    if (ringToShow < 0) ringToShow = 0;
    if (ringToShow >= maxRings) ringToShow = maxRings - 1;
    
    // Light up only the selected ring
    for (int i = 0; i < NUM_LEDS; i++) {
        int ring = getRingForLED(i);
        if (ring == ringToShow) {
            // Use palette for color variation around the ring
            int ledsInRing = getLEDsInRing(ring);
            int firstLED = getFirstLEDInRing(ring);
            int positionInRing = i - firstLED;
            
            uint8_t colorIndex = (positionInRing * 255 / ledsInRing);
            uint8_t ringBrightness = ringIntensity;
            ringBrightness = scale8(ringBrightness, brightness);
            
            CRGB color = ColorFromPalette(currentPalette, colorIndex, ringBrightness, LINEARBLEND);
            leds[i] = color;
        }
    }
}

// Breathing Effect - smooth fade in/out using palette colors
void breathingEffect(uint8_t brightness) {
    static uint32_t lastUpdate = 0;
    static float breathPhase = 0.0; // 0.0 to 1.0 (0 = exhaled, 1 = inhaled)
    static float breathSpeed = 0.0;
    static uint32_t holdStart = 0;
    static bool isHolding = false;
    
    uint32_t now = millis();
    
    // Handle breath hold at peak
    if (isHolding) {
        // Calculate hold duration (breathHoldTime 0-100 maps to 0-2000ms)
        uint32_t holdDuration = (breathHoldTime * 20);
        
        if (now - holdStart > holdDuration) {
            isHolding = false;
            breathPhase = 0.0; // Start exhaling
        } else {
            breathPhase = 1.0; // Keep at peak
        }
    } else {
        // Update breathing phase
        if (now - lastUpdate > 50) { // Update every 50ms
            lastUpdate = now;
            
            // Base speed from breathingRate (1-100, higher = faster)
            // Map to reasonable speed: 1 = slow (0.001), 100 = fast (0.01)
            float baseSpeed = ((float)breathingRate / 100.0) * 0.009 + 0.001;
            
            // Add random variability
            float variability = ((float)breathingVariability / 100.0) * (random8(200) - 100) / 10000.0;
            breathSpeed = baseSpeed + variability;
            
            // Update breath phase
            breathPhase += breathSpeed;
            
            // Check if we've reached peak
            if (breathPhase >= 1.0) {
                breathPhase = 1.0;
                if (breathHoldTime > 0) {
                    isHolding = true;
                    holdStart = now;
                } else {
                    breathPhase = 0.0; // No hold, cycle immediately
                }
            }
            
            // Clamp phase
            if (breathPhase < 0.0) breathPhase = 0.0;
            if (breathPhase > 1.0) breathPhase = 1.0;
        }
    }
    
    // Calculate brightness using smooth curve (ease in/out)
    // Use sine wave for smooth breathing: sin(phase * PI) gives 0->1->0
    float breathBrightness = sin(breathPhase * PI);
    
    // Apply to all LEDs using palette
    for (int i = 0; i < NUM_LEDS; i++) {
        // Use palette with position-based color variation
        uint8_t colorIndex = (i * 255 / NUM_LEDS);
        uint8_t ledBrightness = breathBrightness * ringIntensity;
        ledBrightness = scale8(ledBrightness, brightness);
        
        CRGB color = ColorFromPalette(currentPalette, colorIndex, ledBrightness, LINEARBLEND);
        leds[i] = color;
    }
}

void setAll(CRGB color) {
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = color;
    }
    FastLED.show();
}

void fadeToBlack(int ledNo, byte fadeValue) {
    leds[ledNo].fadeToBlackBy(fadeValue);
}

void meteorRain(CRGB color, byte meteorSize, byte meteorTrailDecay, bool meteorRandomDecay, int SpeedDelay) {
    setAll(CRGB::Black);

    for(int i = 0; i < NUM_LEDS + meteorSize; i++) {
        // Fade brightness all LEDs one step
        for(int j = 0; j < NUM_LEDS; j++) {
            if((!meteorRandomDecay) || (random(10) > 5)) {
                fadeToBlack(j, meteorTrailDecay);        
            }
        }
        
        // Draw meteor
        for(int j = 0; j < meteorSize; j++) {
            if((i - j < NUM_LEDS) && (i - j >= 0)) {
                leds[i - j] = color;
            } 
        }
   
        FastLED.show();
        delay(SpeedDelay);
    }
}

// Set three random LEDs to the specified colors
void glitter(CRGB color1, CRGB color2, CRGB color3){
  // set random LED to white
  int pos1 = random16(NUM_LEDS);
  if (pos1 >= 0 && pos1 < NUM_LEDS) {
    leds[pos1] = color1;
  }

  // set random LED to red
  int pos2 = random16(NUM_LEDS);
  if (pos2 >= 0 && pos2 < NUM_LEDS) {
    leds[pos2] = color2;
  }

  // set random LED to red
  int pos3 = random16(NUM_LEDS);
  if (pos3 >= 0 && pos3 < NUM_LEDS) {
    leds[pos3] = color3;
  }
}

// Helper function that blends one uint8_t toward another by a given amount
void nblendU8TowardU8(uint8_t &cur, const uint8_t target, uint8_t amount)
{
  if (cur == target)
    return;

  if (cur < target)
  {
    uint8_t delta = target - cur;
    delta = scale8_video(delta, amount);
    cur += delta;
  }
  else
  {
    uint8_t delta = cur - target;
    delta = scale8_video(delta, amount);
    cur -= delta;
  }
}

// Blend one CRGB color toward another CRGB color by a given amount.
CRGB fadeTowardColor(CRGB &cur, const CRGB &target, uint8_t amount)
{
  nblendU8TowardU8(cur.red, target.red, amount);
  nblendU8TowardU8(cur.green, target.green, amount);
  nblendU8TowardU8(cur.blue, target.blue, amount);
  return cur;
}

// Fade an entire array of CRGBs toward a given background color by a given amount
void fadeTowardColor(CRGB *L, uint16_t N, const CRGB &bgColor, uint8_t fadeAmount)
{
  for (uint16_t i = 0; i < N; i++)
  {
    fadeTowardColor(L[i], bgColor, fadeAmount);
  }
}

// Function to convert a single hexadecimal character to an integer
uint8_t hexCharToUint(char hexChar)
{
  if (hexChar >= '0' && hexChar <= '9')
  {
    return hexChar - '0';
  }
  else if (hexChar >= 'A' && hexChar <= 'F')
  {
    return 10 + (hexChar - 'A');
  }
  else if (hexChar >= 'a' && hexChar <= 'f')
  {
    return 10 + (hexChar - 'a');
  }
  else
  {
    return 0;
  }
}

// Function to convert a two-character hexadecimal string to an unsigned byte (uint8_t)
uint8_t hexStringToUint8(String hexString)
{
  return 16 * hexCharToUint(hexString.charAt(0)) + hexCharToUint(hexString.charAt(1));
}

// Function to convert a hex color string to a CRGB object
CRGB hexToCRGB(String hexColor)
{
  if (hexColor.length() < 6)
  {
    // Return black or some default color if the string is too short
    return CRGB::Black;
  }

  uint8_t r = hexStringToUint8(hexColor.substring(1, 3));
  uint8_t g = hexStringToUint8(hexColor.substring(3, 5));
  uint8_t b = hexStringToUint8(hexColor.substring(5, 7));

  return CRGB(r, g, b);
}

// Audio amplitude visualization - lights LEDs from bottom to top based on amplitude
void audioAmplitudeEffect(uint8_t brightness) {
  FastLED.setBrightness(brightness);
  
  // Get current amplitude (RMS value)
  static float smoothedAmplitude = 0.0;
  float amplitude = (float)getAmplitude();
  
  // Apply smoothing
  smoothedAmplitude = smoothedAmplitude * audioSmoothing + amplitude * (1.0 - audioSmoothing);
  
  // Use configurable amplitude thresholds
  float minAmplitude = audioMinAmplitude;
  float maxAmplitude = audioMaxAmplitude;
  
  // Clamp amplitude
  if (smoothedAmplitude < minAmplitude) smoothedAmplitude = minAmplitude;
  if (smoothedAmplitude > maxAmplitude) smoothedAmplitude = maxAmplitude;
  
  // Apply normalization
  float normalizedAmplitude = (smoothedAmplitude - minAmplitude) / (maxAmplitude - minAmplitude);
  
  // Use square root scaling for more linear visual response
  // This makes the visualization more impactful and responsive
  normalizedAmplitude = sqrt(normalizedAmplitude);
  
  // Apply additional scaling to make it more impactful
  // Multiply by 1.2 to push values higher, then clamp
  normalizedAmplitude = normalizedAmplitude * 1.2;
  if (normalizedAmplitude > 1.0) normalizedAmplitude = 1.0;
  
  // Map to LED count (0 to NUM_LEDS)
  // Start from a small baseline for quiet sounds, but make it very responsive
  int minLEDs = NUM_LEDS / 20;  // Small baseline (5% of LEDs) for quiet sounds
  int maxLEDs = NUM_LEDS;
  int ledCount = minLEDs + (int)(normalizedAmplitude * (maxLEDs - minLEDs));
  
  // Ensure ledCount is within bounds
  if (ledCount < 0) ledCount = 0;
  if (ledCount > NUM_LEDS) ledCount = NUM_LEDS;
  
  // Clear all LEDs
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  
  // Light up LEDs from bottom to top
  // Use a gradient from color1 (bottom) to color2 (top)
  for (int i = 0; i < ledCount && i < NUM_LEDS; i++) {
    // Create gradient effect
    uint8_t blendAmount = (ledCount > 1) ? map(i, 0, ledCount - 1, 0, 255) : 0;
    leds[i] = blend(color1, color2, blendAmount);
  }
  
  // Add a peak indicator at the top with brighter color
  if (ledCount > 0 && ledCount < NUM_LEDS) {
    leds[ledCount] = color3;
  }
  
  // Add a subtle glow effect for the top few LEDs
  if (ledCount > 3) {
    int startGlow = (ledCount - 3 > 0) ? ledCount - 3 : 0;
    for (int i = startGlow; i < ledCount && i < NUM_LEDS; i++) {
      leds[i] = blend(leds[i], color3, 64);  // Add some color3 glow
    }
  }
}

// FFT Equalizer effect - maps frequency bands to different sections of the LED strip
void fftEqualizerEffect(uint8_t brightness) {
  FastLED.setBrightness(brightness);
  
  // Get FFT bands
  double bands[FFT_BANDS];
  getFFTBands(bands);
  
  // Clear all LEDs
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  
  // Map each band to a section of LEDs
  int ledsPerBand = NUM_LEDS / FFT_BANDS;
  
  // Color palette for different frequency bands
  // Low frequencies: red/orange, Mid: yellow/green, High: blue/purple
  CRGB bandColors[FFT_BANDS];
  for (int i = 0; i < FFT_BANDS; i++) {
    if (i < FFT_BANDS / 3) {
      // Low frequencies - red to orange
      bandColors[i] = CHSV(map(i, 0, FFT_BANDS / 3, 0, 30), 255, 255);
    } else if (i < 2 * FFT_BANDS / 3) {
      // Mid frequencies - yellow to green
      bandColors[i] = CHSV(map(i, FFT_BANDS / 3, 2 * FFT_BANDS / 3, 60, 120), 255, 255);
    } else {
      // High frequencies - cyan to blue to purple
      bandColors[i] = CHSV(map(i, 2 * FFT_BANDS / 3, FFT_BANDS, 120, 200), 255, 255);
    }
  }
  
  // Draw each band
  for (int band = 0; band < FFT_BANDS; band++) {
    int startLED = band * ledsPerBand;
    int endLED = (band + 1) * ledsPerBand;
    
    // Scale band magnitude to LED height (0 to ledsPerBand)
    // Use more sensitive scaling for better responsiveness
    double normalizedBand = bands[band];
    double maxBandValue = 80.0;  // Increased max value to accommodate boosted scaling
    if (normalizedBand > maxBandValue) normalizedBand = maxBandValue;
    if (normalizedBand < 0) normalizedBand = 0;
    
    // Apply square root scaling for more linear visual response
    // This makes smaller values more visible
    double scaledBand = sqrt(normalizedBand / maxBandValue) * maxBandValue;
    
    // Map to LED height with very low threshold for maximum sensitivity
    int minHeight = ledsPerBand / 8;  // Minimum 12.5% of band height for quiet sounds
    int bandHeight = 0;
    
    if (scaledBand > 1.0) {  // Very low threshold - show something for any signal
      // Map with minimum baseline
      bandHeight = minHeight + (int)((scaledBand / maxBandValue) * (ledsPerBand - minHeight));
      if (bandHeight > ledsPerBand) bandHeight = ledsPerBand;
      if (bandHeight < minHeight) bandHeight = minHeight;
    } else if (scaledBand > 0.1) {
      // Even for very quiet sounds, show minimum height
      bandHeight = minHeight;
    }
    
    // Draw the band from bottom to top
    for (int i = 0; i < bandHeight && (startLED + i) < NUM_LEDS && (startLED + i) < endLED; i++) {
      leds[startLED + i] = bandColors[band];
    }
    
    // Add a peak indicator
    if (bandHeight > 0 && bandHeight < ledsPerBand && (startLED + bandHeight) < NUM_LEDS) {
      leds[startLED + bandHeight] = blend(bandColors[band], CRGB::White, 128);
    }
  }
}

// Audio Color Spectrum Effect - Uses amplitude to create a dynamic color-shifting effect
void audioColorSpectrumEffect(uint8_t brightness) {
  FastLED.setBrightness(brightness);
  
  static uint8_t baseHue = 0;  // Base hue that shifts with amplitude
  static float smoothedAmplitude = 0.0;  // Smoothed amplitude for smoother transitions
  static uint32_t lastSparkleTime = 0;  // For sparkle timing
  static int sparklePositions[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};  // Track sparkle positions (-1 = inactive)
  static uint8_t sparkleHues[10] = {0};  // Track sparkle colors
  
  // Get current amplitude (RMS value)
  float amplitude = (float)getAmplitude();
  
  // Smooth the amplitude using configurable smoothing
  smoothedAmplitude = smoothedAmplitude * audioSmoothing + amplitude * (1.0 - audioSmoothing);
  
  // Normalize amplitude using configurable thresholds
  float minAmplitude = audioMinAmplitude;
  float maxAmplitude = audioMaxAmplitude;
  if (smoothedAmplitude < minAmplitude) smoothedAmplitude = minAmplitude;
  if (smoothedAmplitude > maxAmplitude) smoothedAmplitude = maxAmplitude;
  
  float normalizedAmplitude = (smoothedAmplitude - minAmplitude) / (maxAmplitude - minAmplitude);
  normalizedAmplitude = sqrt(normalizedAmplitude);  // Square root for better visual response
  
  // Map amplitude to hue shift using configurable color speed
  // Higher amplitude = faster color shifting
  uint8_t hueShift = (uint8_t)(normalizedAmplitude * (audioColorSpeed / 10.0));  // Scale color speed
  baseHue += hueShift;
  
  // Calculate the number of LEDs to light based on amplitude
  int minLEDs = NUM_LEDS / 10;  // 10% baseline
  int maxLEDs = NUM_LEDS;
  int ledCount = minLEDs + (int)(normalizedAmplitude * (maxLEDs - minLEDs));
  if (ledCount > NUM_LEDS) ledCount = NUM_LEDS;
  
  // Fade all LEDs slightly for trailing effect
  fadeToBlackBy(leds, NUM_LEDS, 20);
  
  // Create a pulsing wave effect that travels up the tree
  // Wave speed uses configurable parameter
  static float wavePosition = 0.0;
  float waveSpeedBase = 0.05;
  float waveSpeedMultiplier = (audioWaveSpeed / 100.0) * 0.3;  // Scale wave speed
  wavePosition += waveSpeedBase + (normalizedAmplitude * waveSpeedMultiplier);
  if (wavePosition > 1.0) wavePosition -= 1.0;
  
  // Draw the main color wave
  for (int i = 0; i < ledCount && i < NUM_LEDS; i++) {
    // Calculate position in wave (0.0 to 1.0)
    float ledPos = (float)i / (float)NUM_LEDS;
    
    // Create wave pattern using sine
    float wavePhase = (ledPos + wavePosition) * 2.0 * PI;
    float waveIntensity = (sin(wavePhase) + 1.0) / 2.0;  // 0.0 to 1.0
    
    // Calculate hue based on position and amplitude
    // Base hue shifts with amplitude, plus position-based variation
    uint8_t hue = baseHue + (uint8_t)(ledPos * 60.0) + (uint8_t)(normalizedAmplitude * 40.0);
    
    // Calculate saturation - higher amplitude = more saturated
    uint8_t saturation = 200 + (uint8_t)(normalizedAmplitude * 55.0);
    
    // Calculate brightness - use wave intensity and amplitude
    uint8_t value = (uint8_t)(waveIntensity * 200.0 * (0.5 + normalizedAmplitude * 0.5));
    value = constrain(value, 50, 255);  // Keep it visible
    
    // Create color
    CRGB color = CHSV(hue, saturation, value);
    
    // Blend with existing color for smooth transitions
    leds[i] = blend(leds[i], color, 200);
  }
  
  // Add sparkles that react to amplitude peaks
  if (normalizedAmplitude > 0.3) {
    // More sparkles with higher amplitude
    int sparkleCount = (int)(normalizedAmplitude * 8.0);
    if (sparkleCount > 8) sparkleCount = 8;
    
    for (int s = 0; s < sparkleCount; s++) {
      // Randomly add new sparkles
      if (random8() < 5) {
        sparklePositions[s] = random16(NUM_LEDS);
        sparkleHues[s] = baseHue + random8(60);  // Random hue near base
      }
      
      // Draw sparkle
      if (sparklePositions[s] >= 0 && sparklePositions[s] < NUM_LEDS) {
        // Scale brightness properly - use 200 instead of 255 for sparkles
        uint8_t sparkleBrightness = scale8(200, brightness);
        leds[sparklePositions[s]] = CHSV(sparkleHues[s], 200, sparkleBrightness);
        
        // Fade out sparkles over time
        if (random8() < 10) {
          sparklePositions[s] = -1;  // Remove sparkle
        }
      }
    }
  }
  
  // Add a bright peak indicator at the top when amplitude is high
  if (normalizedAmplitude > 0.7 && ledCount > NUM_LEDS * 0.9) {
    // Create a bright flash at the top
    for (int i = NUM_LEDS - 5; i < NUM_LEDS; i++) {
      if (i >= 0) {
        // Scale brightness properly
        uint8_t peakBrightness = scale8(200, brightness);  // Use 200 instead of 255
        CRGB peakColor = CHSV(baseHue + 128, 255, peakBrightness);  // Complementary color
        leds[i] = blend(leds[i], peakColor, 150);
      }
    }
  }
  
  // Add a subtle brightness gradient from bottom to top (bottom slightly dimmer)
  for (int i = 0; i < NUM_LEDS; i++) {
    float gradientPos = (float)i / (float)NUM_LEDS;
    // Slightly dim the bottom LEDs for depth effect
    if (gradientPos < 0.3) {
      leds[i].fadeToBlackBy(30);  // Fade bottom 30% slightly
    }
  }
}

// Calculate auto brightness based on amplitude
// Maps amplitude between minAmplitude and maxAmplitude to brightness between minBrightness and maxBrightness
uint8_t calculateAutoBrightness(uint8_t baseBrightness) {
  // If auto brightness is disabled, return the base brightness
  if (!autoBrightnessEnabled) {
    return baseBrightness;
  }
  
  // Get current amplitude
  float amplitude = (float)getAmplitude();
  
  // Clamp amplitude to the configured range
  if (amplitude < autoBrightnessMinAmplitude) {
    amplitude = autoBrightnessMinAmplitude;
  }
  if (amplitude > autoBrightnessMaxAmplitude) {
    amplitude = autoBrightnessMaxAmplitude;
  }
  
  // Normalize amplitude to 0.0-1.0 range
  float amplitudeRange = autoBrightnessMaxAmplitude - autoBrightnessMinAmplitude;
  if (amplitudeRange <= 0.0) {
    // If range is invalid, return min brightness
    return autoBrightnessMinBrightness;
  }
  
  float normalizedAmplitude = (amplitude - autoBrightnessMinAmplitude) / amplitudeRange;
  
  // Map normalized amplitude to brightness range
  float brightnessRange = (float)autoBrightnessMaxBrightness - (float)autoBrightnessMinBrightness;
  float calculatedBrightness = (float)autoBrightnessMinBrightness + (normalizedAmplitude * brightnessRange);
  
  // Clamp to ensure we never go below min or above max
  uint8_t result = (uint8_t)calculatedBrightness;
  if (result < autoBrightnessMinBrightness) {
    result = autoBrightnessMinBrightness;
  }
  if (result > autoBrightnessMaxBrightness) {
    result = autoBrightnessMaxBrightness;
  }
  
  return result;
}