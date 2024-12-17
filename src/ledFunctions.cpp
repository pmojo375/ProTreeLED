#include <ledFunctions.h>

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

void risingSparklesEffect() {
    const uint8_t sparkleChance = 25; // Chance of new sparkle
    const uint8_t sparkleFade = 1;  // Sparkle fade out speed

    // Shift everything up one pixel per frame
    for (int i = 0; i < NUM_LEDS - 1; i++) {
        leds[i] = leds[i + 1];
    }

    // Randomly create new sparkle at the bottom
    if (random8() < sparkleChance) {
        leds[NUM_LEDS - 1] = CHSV(random8(160, 200), 255, 255); // Cool color range
    } else {
        leds[NUM_LEDS - 1] = CRGB::Black;
    }

    // Fade the LEDs every 5 frames
    if (millis() % 5 == 0) {
     fadeToBlackBy(leds, NUM_LEDS, sparkleFade);
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
  leds[random16(NUM_LEDS)] = color1;

  // set random LED to red
  leds[random16(NUM_LEDS)] = color2;

  // set random LED to red
  leds[random16(NUM_LEDS)] = color3;
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