#include <FastLED.h>
#include "constants.h"
#include "CakePalettes.h"
#include "patterns/comet.h"
#include "patterns/Monochrome.h"
 
#define LED_PIN     10
#define NUM_LEDS    120
#define BRIGHTNESS  64
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
 
#define UPDATES_PER_SECOND 200
#define GROUP_SIZE (NUM_LEDS / 4)

CRGB group1[GROUP_SIZE];
CRGB group2[GROUP_SIZE];
CRGB group3[GROUP_SIZE];
CRGB group4[GROUP_SIZE];
 
CRGBPalette16 currentPalette;
TBlendType    currentBlending;
 
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;
 
void setup() {
    delay( 3000 ); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    
    currentPalette = CloudColors_p;
    currentBlending = LINEARBLEND;
    MySetup();
}

void MySetup() {
    // Copy the relevant LEDs to each group
    memcpy(group1, leds, GROUP_SIZE * sizeof(CRGB));
    memcpy(group2, leds + GROUP_SIZE, GROUP_SIZE * sizeof(CRGB));
    memcpy(group3, leds + 2 * GROUP_SIZE, GROUP_SIZE * sizeof(CRGB));
    memcpy(group4, leds + 3 * GROUP_SIZE, GROUP_SIZE * sizeof(CRGB));
}
 
void loop() {
    Monochrome monochrome = Monochrome(CRGB::Blue, NUM_LEDS);
    monochrome.run();
}
 
void dep_loop()
{
    CRGB color = currentPalette[currentLED % 16];
    // Run your patterns on each group
    runPatternOnGroup(group1, GROUP_SIZE, currentDirection, color);
    runPatternOnGroup(group2, GROUP_SIZE, currentDirection * -1, color);
    runPatternOnGroup(group3, GROUP_SIZE, currentDirection * -1, color);
    runPatternOnGroup(group4, GROUP_SIZE, currentDirection, color);
  
    // Copy the groups back to the main LED array
    memcpy(leds, group1, GROUP_SIZE * sizeof(CRGB));
    memcpy(leds + GROUP_SIZE, group2, GROUP_SIZE * sizeof(CRGB));
    memcpy(leds + 2 * GROUP_SIZE, group3, GROUP_SIZE * sizeof(CRGB));
    memcpy(leds + 3 * GROUP_SIZE, group4, GROUP_SIZE * sizeof(CRGB));
  
    FastLED.show();
    
    // Move to the next LED
    currentLED = (currentLED + 1) % GROUP_SIZE;
    delay(30);

    if (currentLED == 0) {
        ClearLedGroup();
        currentLED = 0;

        if (currentDirection == FORWARD) {
            currentDirection = BACKWARD;
        } else {
            currentDirection = FORWARD;
        }
    }
}

void ClearLedGroup() {
    for (int i = 0; i < GROUP_SIZE; i++) {
        group1[i] = CRGB::Black;
        group2[i] = CRGB::Black;
        group3[i] = CRGB::Black;
        group4[i] = CRGB::Black;
    }
    FastLED.show();
}
 
void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    
    for( int i = 0; i < NUM_LEDS; ++i) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}
 
// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.  All are shown here.
 
void ChangePalettePeriodically()
{

    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;

    

    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        if( secondHand ==  0)  { currentPalette = RainbowColors_p;               currentBlending = LINEARBLEND; }
        if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;         currentBlending = NOBLEND;  }
        if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;         currentBlending = LINEARBLEND; }
        if( secondHand == 20)  { currentPalette = SetupPurpleAndGreenPalette();  currentBlending = LINEARBLEND; }
        if( secondHand == 25)  { currentPalette = SetupTotallyRandomPalette();                    currentBlending = LINEARBLEND; }
        if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();             currentBlending = NOBLEND; }
        if( secondHand == 35)  { SetupBlackAndWhiteStripedPalette();             currentBlending = LINEARBLEND; }
        if( secondHand == 40)  { currentPalette = CloudColors_p;                 currentBlending = LINEARBLEND; }
        if( secondHand == 45)  { currentPalette = PartyColors_p;                 currentBlending = LINEARBLEND; }
        if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p;       currentBlending = NOBLEND;  }
        if( secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p;       currentBlending = LINEARBLEND; }
    }
}
 
// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};
 
 
 
// Additional notes on FastLED compact palettes:
//
// Normally, in computer graphics, the palette (or "color lookup table")
// has 256 entries, each containing a specific 24-bit RGB color.  You can then
// index into the color palette using a simple 8-bit (one byte) value.
// A 256-entry color palette takes up 768 bytes of RAM, which on Arduino
// is quite possibly "too many" bytes.
//
// FastLED does offer traditional 256-element palettes, for setups that
// can afford the 768-byte cost in RAM.
//
// However, FastLED also offers a compact alternative.  FastLED offers
// palettes that store 16 distinct entries, but can be accessed AS IF
// they actually have 256 entries; this is accomplished by interpolating
// between the 16 explicit entries to create fifteen intermediate palette
// entries between each pair.
//
// So for example, if you set the first two explicit entries of a compact 
// palette to Green (0,255,0) and Blue (0,0,255), and then retrieved 
// the first sixteen entries from the virtual palette (of 256), you'd get
// Green, followed by a smooth gradient from green-to-blue, and then Blue.