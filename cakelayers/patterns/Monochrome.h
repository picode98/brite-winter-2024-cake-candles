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
        unsigned long lastUpdate = 0;
        while(true) {
            unsigned long currentMillis = millis();
            if (currentMillis - lastUpdate < delay) {
                continue;
            }

            for (int i = 0; i < numGroups; i++) {
                for (int j = 0; j < groupSizes[i]; j++) {
                    groups[i][j] = palette[i % 16];
                }
            }

            // Copy the data from groups to leds
            for (int i = 0; i < numGroups; i++) {
                memcpy(leds + i * groupSizes[i], groups[i], groupSizes[i] * sizeof(CRGB));
            }

            FastLED.show();
        }
    }
};

#endif // MONOCHROME_H