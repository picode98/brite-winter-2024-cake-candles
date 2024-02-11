// #include <bitswap.h>
// #include <chipsets.h>
// #include <color.h>
// #include <colorpalettes.h>
// #include <colorutils.h>
// #include <controller.h>
// #include <cpp_compat.h>
// #include <dmx.h>
// #include <fastled_config.h>
// #include <fastled_delay.h>
// #include <fastled_progmem.h>
#include <FastLED.h>

#include <esp_mac.h>
// #include <fastpin.h>
// #include <fastspi_bitbang.h>
// #include <fastspi_dma.h>
// #include <fastspi_nop.h>
// #include <fastspi_ref.h>
// #include <fastspi_types.h>
// #include <fastspi.h>
// #include <hsv2rgb.h>
// #include <led_sysdefs.h>
// #include <lib8tion.h>
// #include <noise.h>
// #include <pixelset.h>
// #include <pixeltypes.h>
// #include <platforms.h>
// #include <power_mgt.h>

const int DEPLOYED_VERSION = 4;

const int DATA_PIN = 12;

// Array of "orbit" (LED chase sequence) configurations, where the indices
// are [animation][orbit][LED] and the values are LED indices (on the physical
// strand and in CURRENT_COLORS).

struct OrbitDefinition
{
    size_t numOrbits, ledsPerOrbit;
    size_t** orbits;
};

// const int TOP_BOTTOM_ORBIT[2][4] = {
//     {2, 3, 4, 6}, // top
//     {0, 7, 5, 1}  // bottom
// };

// const int LONG_ORBIT[1][]

struct OrbitDef
{
    double runTime, hueFrequency;
    double* frequencies;
    const int** orbitIndices;
};

// OrbitDef foo = {10.0, 0.01, (double[]) {0.5, 1.0, -1.0}, (const int* []) {
//         (const int[]) {2, 3, 4, 6, -1}, // top
//         (const int[]) {0, 7, 5, 1, -1},  // bottom
//         (const int[]) {-1}
// }};

OrbitDef ORBITS[] = {
    OrbitDef {10.0, 0.01, (double[]) {0.5, 1.0, -1.0}, (const int* []) {
        (const int[]) {2, 3, 4, 6, -1}, // top
        (const int[]) {0, 7, 5, 1, -1},  // bottom
        (const int[]) {-1}
    }},
    OrbitDef {10.0, 0.02, (double[]) {0.5, 1.0, -1.0}, (const int* []) {
        (const int[]) {1, 5, 7, 0, -1}, // bottom
        (const int[]) {6, 4, 3, 2, -1},  // top
        (const int[]) {-1}
    }},
    OrbitDef {10.0, 0.04, (double[]) {0.5, 1.0, -1.0}, (const int* []) {
        (const int[]) {1, 3, 4, 5, -1}, // right
        (const int[]) {7, 0, 2, 6, -1},  // left
        (const int[]) {-1}
    }},
    OrbitDef {10.0, 0.08, (double[]) {0.5, 1.0, -1.0}, (const int* []) {
        (const int[]) {6, 2, 0, 7, -1}, // left
        (const int[]) {5, 4, 3, 1, -1},  // right
        (const int[]) {-1}
    }},
    OrbitDef {10.0, 0.16, (double[]) {0.5, 1.0, -1.0}, (const int* []) {
        (const int[]) {0, 1, 4, 6, -1}, // diagonals
        (const int[]) {7, 5, 3, 2, -1},
        (const int[]) {-1}
    }},
    OrbitDef {10.0, 0.32, (double[]) {0.5, 1.0, -1.0}, (const int* []) {
        (const int[]) {2, 3, 5, 7, -1}, // diagonals (opposite direction)
        (const int[]) {6, 4, 1, 0, -1},
        (const int[]) {-1}
    }},
    OrbitDef {15000.0, 0.0, (double[]) {0.2, -1.0}, (const int* []) {
        (const int[]) {0, 2, 3, 1, 0, 2, 3, 1, 0, 2, 6, 7, 0, 2, 6, 7, 5, 4, 6, 7, 5, 4, 6, 7, 5, 4, 3, 1, 5, 4, 3, 1, -1},
        (const int[]) {-1}
    }}
};
const size_t NUM_ANIMATIONS = sizeof(ORBITS) / sizeof(ORBITS[0]),
             MAX_NUM_ORBITS = 2; //, NUM_ORBITS = sizeof(ORBITS[0]) / sizeof(ORBITS[0][0]), ORBIT_SIZE = sizeof(ORBITS[0][0]) / sizeof(ORBITS[0][0][0]);


double /* ORBIT_FREQUENCIES[MAX_NUM_ORBITS] = {1.5, 0.1}, */ ORBIT_PHASES[MAX_NUM_ORBITS] = {0.0};
// double HUE_FREQUENCY = 0.01;
double ANIM_LENGTHS[NUM_ANIMATIONS] = {0.0}, CURRENT_ANIM_TIME = 0.0;
bool INVERT_BRIGHTNESS = false;

const size_t NUM_LEDS = 8;
CRGB CURRENT_COLORS[NUM_LEDS];
size_t CURRENT_ANIMATION = 0;
double HUE_PHASE = 0.0;
unsigned long PREVIOUS_MILLIS;

double getBrightness(size_t orbit, double phase) // , double animTime, double animLength, bool invert)
{
    // double animTransitionTerm = min(max((min(animTime, animLength - animTime) - 0.25) * 4.0, 0.0), 1.0); // Term that fades out between animations

    // double brightness = 1.0;
    switch(orbit)
    {
        case 0: // Jump to full brightness, then fade quickly.
            return max(1.0 - 8.0 * phase, 0.0);
            break;
        case 1: // Jump to full brightness, then fade slowly until phase = 0.6, then fade quickly.
            return max((phase <= 0.6 ? 1.0 - 0.75 * phase : (1.0 - 0.75 * 0.6) - 2.0 * (phase - 0.6)), 0.0);
            break;
        default:
            return 1.0;
    }

    // return animTransitionTerm * (invert ? (1.0 - brightness) : brightness);
    // return (orbit == 0 ? max(1.0 - 2.0 * phase, 0.0) : max(1.0 - 0.75 * phase, 0.0));
}

void setup()
{
    // Serial.begin(9600);

    // The built-in LED is used as a status indicator.
    pinMode(LED_BUILTIN, OUTPUT);
    for(size_t i = 0; i < NUM_LEDS; ++i)
    {
        CURRENT_COLORS[i] = CRGB(0x000000);
    }

    // The light strings we're using are NeoPixel-compatible, but seem to use
    // a different color order (RGB) than the default (GRB).
    FastLED.addLeds<WS2812, DATA_PIN, EOrder::RGB>(CURRENT_COLORS, NUM_LEDS);

    // Blink the current version on the status LED and one of the string LEDs.
    // This is meant as an easy way to tell which version has been deployed on a
    // controller or whether a particular controller has been updated.
    for(size_t i = 0; i < DEPLOYED_VERSION; ++i)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        CURRENT_COLORS[0] = CRGB(0xffffff);
        FastLED.show();
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        CURRENT_COLORS[0] = CRGB(0x000000);
        FastLED.show();
        delay(500);
    }

    // Calculate a random seed using the MAC address of the built-in radio.
    // This is meant to give each controller's effects a distinct "personality",
    // which persists even if power cycled.
    uint8_t macAddress[8] = {0};
    esp_efuse_mac_get_default(macAddress);
    unsigned long seed = 0;
    for(size_t i = 0; i < sizeof(macAddress) / sizeof(unsigned long); ++i)
    {
        seed += reinterpret_cast<unsigned long*>(macAddress)[i];
    }
    randomSeed(seed + 1);

    for(size_t anim = 0; anim < NUM_ANIMATIONS; ++anim)
    {
        ORBITS[anim].runTime *= (1.5 * (random() / static_cast<double>(RAND_MAX)) + 0.5);
        ORBITS[anim].hueFrequency *= (1.5 * (random() / static_cast<double>(RAND_MAX)) + 0.5);

        for(size_t orbit = 0; ORBITS[anim].frequencies[orbit] > 0.0; ++orbit)
        {
            ORBITS[anim].frequencies[orbit] *= (1.0 * (random() / static_cast<double>(RAND_MAX)) + 0.5);
        }
    }

    // Calculate random frequencies for the individual orbits and for the hue.
    // double minFrequency = -1.0;
    // for(size_t i = 0; i < MAX_NUM_ORBITS; ++i)
    // {
    //     ORBIT_FREQUENCIES[i] = 2.0 * (random() / static_cast<double>(RAND_MAX)) + 0.1;
    //     if(minFrequency < 0.0 || ORBIT_FREQUENCIES[i] < minFrequency)
    //     {
    //         minFrequency = ORBIT_FREQUENCIES[i];
    //     }
    // }
    // HUE_FREQUENCY = 0.02 * (random() / static_cast<double>(RAND_MAX)) + 0.005;

    // Each animation should last at least twice the period of the longest orbit, with
    // a random amount of additional time.
    // double minDuration = 2.0 / minFrequency; // , baseTime = 0.0;
    // for(size_t i = 0; i < NUM_ANIMATIONS; ++i)
    // {
    //     ANIM_LENGTHS[i] = minDuration + 5.0 * (random() / static_cast<double>(RAND_MAX));
    //     // baseTime = ANIM_END_TIMES[i];
    // }

    PREVIOUS_MILLIS = millis();
}

void loop()
{
    // Update oscillator phases, which run from 0.0 to 1.0.
    unsigned long newMillis = millis();
    double delta = (newMillis - PREVIOUS_MILLIS) / 1000.0;
    for(size_t i = 0; ORBITS[CURRENT_ANIMATION].frequencies[i] > 0.0; ++i)
    {
        ORBIT_PHASES[i] += ORBITS[CURRENT_ANIMATION].frequencies[i] * delta;
        // if(ORBIT_PHASES[i] >= 1.0) Serial.write('\n');
        ORBIT_PHASES[i] -= static_cast<int>(ORBIT_PHASES[i]);
    }
    // PHASE_1 += frequency_1 * delta; PHASE_1 -= static_cast<int>(PHASE);
    // PHASE_2 += frequency_2 * delta; PHASE_2 -= static_cast<int>(PHASE);
    HUE_PHASE += ORBITS[CURRENT_ANIMATION].hueFrequency * delta; HUE_PHASE -= static_cast<int>(HUE_PHASE);

    // Update current animation elapsed time, and roll over to the next animation if this
    // one has ended.
    CURRENT_ANIM_TIME += delta;
    while(CURRENT_ANIM_TIME >= ORBITS[CURRENT_ANIMATION].runTime)
    {
        CURRENT_ANIM_TIME -= ORBITS[CURRENT_ANIMATION].runTime;
        CURRENT_ANIMATION = (CURRENT_ANIMATION + 1) % NUM_ANIMATIONS;

        if(CURRENT_ANIMATION == 0)
        {
            INVERT_BRIGHTNESS = !INVERT_BRIGHTNESS;
        }
    }

    PREVIOUS_MILLIS = newMillis;

    // Display "running" status: On for 9 seconds, off for 1 second.
    digitalWrite(LED_BUILTIN, (newMillis % 10000 <= 9000 ? HIGH : LOW));

    // size_t currentAnimation = (newMillis % (5000 * NUM_ANIMATIONS)) / 5000;

    auto thisAnim = ORBITS[CURRENT_ANIMATION];
    double brightnesses[NUM_LEDS] = {0.0}, hues[NUM_LEDS] = {0.0};
    size_t thisNumOrbits = 0;
    while(thisAnim.orbitIndices[thisNumOrbits][0] != -1) ++thisNumOrbits;
    for(size_t orbit = 0; orbit < thisNumOrbits; ++orbit)
    {
        size_t thisOrbitSize = 0;
        while(thisAnim.orbitIndices[orbit][thisOrbitSize] != -1) ++thisOrbitSize;

        for(size_t ledIdx = 0; ledIdx < thisOrbitSize; ++ledIdx)
        {
            size_t led = thisAnim.orbitIndices[orbit][ledIdx];
            // Serial.printf("%d ", led);

            double adjustedPhase = ORBIT_PHASES[orbit] - static_cast<double>(ledIdx) / thisOrbitSize, thisHuePhase = HUE_PHASE - static_cast<double>(orbit) / thisNumOrbits;
            adjustedPhase += static_cast<int>(adjustedPhase < 0.0);
            thisHuePhase += static_cast<int>(thisHuePhase < 0.0);

            hues[led] = thisHuePhase;
            brightnesses[led] += getBrightness(orbit, adjustedPhase); // , CURRENT_ANIM_TIME, ORBITS[CURRENT_ANIMATION].runTime, INVERT_BRIGHTNESS);
            brightnesses[led] = min(brightnesses[led], 1.0);
            // if(brightnesses[led] >= 0.95)
            // {
            //     Serial.printf("%d", led);
            // }

            // if(INVERT_BRIGHTNESS)
            // {
            //     brightness = 1.0 - brightness;
            // }

            // Use opposite-hued colors for the two orbits.
            // if(orbit == 1)
            // {
            //     thisHuePhase += 0.5;
            //     thisHuePhase -= static_cast<int>(thisHuePhase);
            // }
            // CHSV color = CHSV(static_cast<uint8_t>((orbit == 0 ? HUE_PHASE : (1.0 - HUE_PHASE)) * 255), 255, static_cast<uint8_t>(brightness * 255));
            // CRGB color = CRGB(0x000000);
            // (orbit == 0 ? color.red : (currentAnimation == 0 ? color.blue : color.green)) = static_cast<uint8_t>(brightness * 255);
            
            // Set the current LED using HSV (converted to RGB), scaling the hue phase and brightness from [0, 1] to [0, 255].
            // CURRENT_COLORS[ORBITS[CURRENT_ANIMATION][orbit][led]].setHSV(static_cast<uint8_t>(thisHuePhase * 255), 255, static_cast<uint8_t>(brightness * 255));
        }
    }

    double animTransitionTerm = min(max((min(CURRENT_ANIM_TIME, ORBITS[CURRENT_ANIMATION].runTime - CURRENT_ANIM_TIME) - 0.25) * 4.0, 0.0), 1.0);
    for(size_t i = 0; i < NUM_LEDS; ++i)
    {
        CURRENT_COLORS[i].setHSV(static_cast<uint8_t>(hues[i] * 255), 255, static_cast<uint8_t>(animTransitionTerm * (INVERT_BRIGHTNESS ? (1.0 - brightnesses[i]) : brightnesses[i]) * 255));
        // Serial.printf("%f ", brightnesses[i]);
    }
    // Serial.write("\n");

    FastLED.show();

    // for (size_t i = 0; i < NUM_LEDS; i++)
    // {
    //     CURRENT_COLORS[i] = CRGB::Green;
    // }
    // FastLED.show();
    // delay(1000);

    // for (size_t i = 0; i < NUM_LEDS; i++)
    // {
    //     CURRENT_COLORS[i] = CRGB::Red;
    // }
    // FastLED.show();
    // delay(1000);

    delay(10);
}