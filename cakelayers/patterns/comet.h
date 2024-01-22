#ifndef COMET_H
#define COMET_H

#include <FastLED.h>

extern const int GROUP_SIZE;

extern CRGB group1[];
extern CRGB group2[];
extern CRGB group3[];
extern CRGB group4[];

int currentLED = 0;
int currentDirection = FORWARD;

void runPatternOnGroup(CRGB* group, int size, int direction, CRGB color, int tailSize = 16) {
    if (direction > 0) {
        // Move forward
        group[currentLED] = color;
        for (int i = 1; i <= tailSize; i++) {
        if (currentLED - i >= 0) {
            group[currentLED - i].fadeToBlackBy(255 * i / tailSize);
        }
        }
    } else {
            // Move backward
            group[size - currentLED - 1] = color;
            for (int i = 1; i <= tailSize; i++) {
            if (size - currentLED - 1 + i < size) {
                group[size - currentLED - 1 + i].fadeToBlackBy(255 * i / tailSize);
            }
        }
    }
}

#endif // COMET_H