#ifndef PATTERN_H
#define PATTERN_H

#include <FastLED.h>

class Pattern {
public:
    // The current palette
    CRGBPalette16 palette;

    // The number of LEDs in the strip
    int numLEDs;

    // The number of groups
    int numGroups;

    // The delay between updates
    int delay;

    // An array to hold the sizes of the groups
    int* groupSizes;

    // An array of pointers to the groups
    CRGB** groups;

    Pattern(CRGBPalette16 palette, int numLEDs, int numGroups = 1, int delay = 1000) : palette(palette), numLEDs(numLEDs), numGroups(numGroups), delay(1000) {
        groupSizes = new int[numGroups];
        groups = new CRGB*[numGroups];

        for (int i = 0; i < numGroups; i++) {
            groupSizes[i] = numLEDs / numGroups;
            groups[i] = new CRGB[groupSizes[i]];
        }
    }

    virtual void clear() {
        for (int i = 0; i < numGroups; i++) {
            for (int j = 0; j < groupSizes[i]; j++) {
                groups[i][j] = CRGB::Black;
            }
        }
    }
};

#endif // PATTERN_H