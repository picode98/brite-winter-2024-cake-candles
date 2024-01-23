/// @file    DemoReel100.ino
/// @brief   FastLED "100 lines of code" demo reel, showing off some effects
/// @example DemoReel100.ino

#include <Keypad.h>
#include <FastLED.h>

FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few 
// of the kinds of animation patterns you can quickly and easily 
// compose using FastLED.  
//
// This example also shows one easy way to define multiple 
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014


#define DATA_PIN    10
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    120
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

void setup() {
  Serial.begin(9600);
  delay(3000); // 3 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

//define the two-dimensional array on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 4, 3, 2}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
// SimplePatternList gPatterns = { divideIntoFourGroups };
// SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm, fourGroupsJuggle };
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm, backToFront, fourGroupsJuggle, twoGroupsPattern };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

enum gUpdateHueModes {
  gUpdateHueModeNone,
  gUpdateHueModeIncrease,
  gUpdateHueModeDecrease
};

int gUpdateHueMode = gUpdateHueModeIncrease;
char lastKeyPressed = 0;
  
void loop()
{
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { 
    updateHue();
    readKeyPad();
  } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void readKeyPad() {
  char key = customKeypad.getKey();
  if (key) {
    lastKeyPressed = key; 
  }
}

void nextPattern()
{
    char key = lastKeyPressed;
    if (key) {
      if (key == '1') {
        gCurrentPatternNumber = 0;
      } else if (key == '2') {
        gCurrentPatternNumber = 1;
      } else if (key == '3') {
        gCurrentPatternNumber = 2;
      } else if (key == '4') {
        gCurrentPatternNumber = 3;
      } else if (key == '5') {
        gCurrentPatternNumber = 4;
      } else if (key == '6') {
        gCurrentPatternNumber = 5;
      } else if (key == '7') {
        gCurrentPatternNumber = 6;
      } else if (key == '8') {
        gCurrentPatternNumber = 7;
      } else if (key == '9') {
        gCurrentPatternNumber = 8;
      } else if (key == '0') {
        gCurrentPatternNumber = 9;
      } else if (key == '*') {
        gUpdateHueMode = gUpdateHueModeIncrease;
      } else if (key == '#') {
        gUpdateHueMode = gUpdateHueModeDecrease;
      } else if (key == 'A') {
        gCurrentPatternNumber = 0;
      } else if (key == 'B') {
        gCurrentPatternNumber = 1;
      } else if (key == 'C') {
        gCurrentPatternNumber = 2;
      } else if (key == 'D') {
        gCurrentPatternNumber = 3;
      }
      lastKeyPressed = 0;
    } else {
      // add one to the current pattern number, and wrap around at the end
      gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);  
    }
}

void updateHue() {
  if (gUpdateHueMode == gUpdateHueModeIncrease) {
    gHue++;
  } else if (gUpdateHueMode == gUpdateHueModeDecrease) {
    gHue--;
  }
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  uint8_t dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void backToFront() {
  // Fade all LEDs to black
  fadeToBlackBy(leds, NUM_LEDS, 20);

  // Define the size of each group
  int groupSize = NUM_LEDS / 4;

  // Define the colors for the dots in each group
  uint8_t dotHues[] = {0, 64, 128, 192};

  // For each group...
  for (int group = 0; group < 4; group++) {
    // Calculate the position of the dot within the group
    int pos;

    // Determine the direction based on the group number
    if (group % 2 == 0) {
        // Even group: Move forward
        pos = beatsin16(7, 0, groupSize - 1);
    } else {
        // Odd group: Move backward
        pos = groupSize - 1 - beatsin16(7, 0, groupSize - 1);
    }

    // Add the position of the group to get the position of the dot within the entire strip
    pos += group * groupSize;

    // Set the color of the dot
    leds[pos] |= CHSV(gHue, 200, 255);
  }
}


void fourGroupsJuggle() {
  // Fade all LEDs to black
  fadeToBlackBy(leds, NUM_LEDS, 20);

  // Define the size of each group
  int groupSize = NUM_LEDS / 4;

  // Define the colors for the dots in each group
  uint8_t dotHues[] = {0, 64, 128, 192};

  // For each group...
  for (int group = 0; group < 4; group++) {
    // For each dot in the group...
    for (int dot = 0; dot < 2; dot++) {
      // Calculate the position of the dot within the group
      int pos = beatsin16(dot + 7, 0, groupSize - 1);

      // Add the position of the group to get the position of the dot within the entire strip
      pos += group * groupSize;

      // Set the color of the dot
      leds[pos] |= CHSV(dotHues[group], 200, 255);
    }
  }
}

void twoGroupsPattern() {
    // Fade all LEDs to black
    fadeToBlackBy(leds, NUM_LEDS, 20);

    // Define the size of each group
    int groupSize = NUM_LEDS / 2;

    // For each group...
    for (int group = 0; group < 2; group++) {
        // Calculate the position of the dot within the group
        int pos;
        if (group == 0) {
            pos = beatsin16(7, 0, groupSize - 1);
        } else {
            pos = groupSize - 1 - beatsin16(7, 0, groupSize - 1);
        }

        // Add the position of the group to get the position of the dot within the entire strip
        pos += group * groupSize;

        // Set the color of the dot
        leds[pos] |= CHSV(gHue, 200, 255);

        // Add a glitter effect near the head of the comet
        for (int i = 0; i < 2; i++) {
            if (random8() < 20) {
                int glitterPos = (pos - i + NUM_LEDS) % NUM_LEDS;  // Change here for the second group
                uint8_t fadeValue = random8();
                leds[glitterPos] = blend(CRGB::White, leds[glitterPos], fadeValue);
            }
        }
    }
}

void divideIntoFourGroups() {
  // Calculate the size of each group
  int groupSize = NUM_LEDS / 4;

  // Define an array to hold the colors for each group
  CRGB groupColors[4];
  unsigned long lastColorChangeTime = millis();
  unsigned long colorChangeInterval = random(500, 1000);

  // Assign a random solid color to each group
  for (int group = 0; group < 4; group++) {
    groupColors[group] = CRGB(random8(), random8(), random8());
  }

  // Loop through all LEDs
  for (int i = 0; i < NUM_LEDS; i++) {
    // Calculate the group index for the current LED
    int group = i / groupSize;

    // Set the color of the LED based on the current group color
    leds[i] = groupColors[group];

    // Check if it's time to change the color
    if (millis() - lastColorChangeTime >= colorChangeInterval) {
      // Select a random group to change to
      int newGroup = random(4);

      // Update the last color change time and color change interval
      lastColorChangeTime = millis();
      colorChangeInterval = random(500, 1000);
    }
  }
  delay(300);
}


