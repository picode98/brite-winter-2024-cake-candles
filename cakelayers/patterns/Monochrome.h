#ifndef MONOCHROME_H
#define MONOCHROME_H

#include <FastLED.h>
#include "Pattern.h"

extern CRGB leds[];

class Monochrome : public Pattern {
public:
    // Constructor
    Monochrome(CRGB color, int numLEDs) 
        : Pattern(CRGBPalette16(color), numLEDs) {
    }

    void run() {
        unsigned long currentMillis = millis();
        for (int i = 0; i < numGroups; i++) {
            for (int j = 0; j < groupSizes[i]; j++) {
                groups[i][j] = palette[i % 16];
            }
        }

        // Copy the data from groups to leds
        for (int i = 0; i < numGroups; i++) {
            memcpy(leds + i * groupSizes[i], groups[i], groupSizes[i] * sizeof(CRGB));
        }
    }
};

#endif // MONOCHROME_H