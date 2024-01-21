#include "FastLED.h"

// This function sets up a palette of purple and green stripes.
CRGBPalette16 SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    return CRGBPalette16(
        green,  green,  black,  black,
        purple, purple, black,  black,
        green,  green,  black,  black,
        purple, purple, black,  black
    );
}

// This function fills the palette with totally random colors.
CRGBPalette16 SetupTotallyRandomPalette()
{
    CRGBPalette16 palatte;
    for( int i = 0; i < 16; ++i) {
        palatte[i] = CHSV( random8(), 255, random8());
    }
    return palatte;
}
 
// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
CRGBPalette16 SetupBlackAndWhiteStripedPalette()
{
    CRGBPalette16 palatte;
    // 'black out' all 16 palette entries...
    fill_solid( palatte, 16, CRGB::Black);
    // and set every fourth one to white.
    palatte[0] = CRGB::White;
    palatte[4] = CRGB::White;
    palatte[8] = CRGB::White;
    palatte[12] = CRGB::White;
    
    return palatte;
}