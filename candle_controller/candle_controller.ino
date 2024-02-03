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

const int DEPLOYED_VERSION = 1;

const int DATA_PIN = 12;
const int ORBITS[][2][4] = {
    {
        {2, 3, 4, 6}, // top
        {0, 7, 5, 1}  // bottom
    },
    {
        {1, 5, 7, 0}, // bottom
        {6, 4, 3, 2}  // top
    },
    {
        {1, 3, 4, 5}, // right
        {7, 0, 2, 6}  // left
    },
    {
        {6, 2, 0, 7}, // left
        {5, 4, 3, 1}  // right
    },
    {
        {0, 1, 4, 6},
        {7, 5, 3, 2}
    },
    {
        {2, 3, 5, 7},
        {6, 4, 1, 0}
    }
};
const size_t NUM_ANIMATIONS = sizeof(ORBITS) / sizeof(ORBITS[0]), NUM_ORBITS = sizeof(ORBITS[0]) / sizeof(ORBITS[0][0]), ORBIT_SIZE = sizeof(ORBITS[0][0]) / sizeof(ORBITS[0][0][0]);

double ORBIT_FREQUENCIES[NUM_ORBITS] = {1.5, 0.1}, ORBIT_PHASES[NUM_ORBITS] = {0.0};
double HUE_FREQUENCY = 0.01;
double ANIM_LENGTHS[NUM_ANIMATIONS] = {0.0}, CURRENT_ANIM_TIME = 0.0;

const size_t NUM_LEDS = 8;
CRGB CURRENT_COLORS[NUM_LEDS];
size_t CURRENT_ANIMATION = 0;
double HUE_PHASE = 0.0;
unsigned long PREVIOUS_MILLIS;

double getBrightness(size_t orbit, double phase, double animTime, double animLength)
{
    double animTransitionTerm = min(max((min(animTime, animLength - animTime) - 0.25) * 4.0, 0.0), 1.0); // Term that fades out between animations

    switch(orbit)
    {
        case 0:
            return animTransitionTerm * max(1.0 - 2.0 * phase, 0.0);
        case 1:
            return animTransitionTerm * max((phase <= 0.6 ? 1.0 - 0.75 * phase : (1.0 - 0.75 * 0.6) - 2.0 * (phase - 0.6)), 0.0);
        default:
            return 1.0;
    }
    // return (orbit == 0 ? max(1.0 - 2.0 * phase, 0.0) : max(1.0 - 0.75 * phase, 0.0));
}

void setup()
{
    for(size_t i = 0; i < NUM_LEDS; ++i)
    {
        CURRENT_COLORS[i] = CRGB(0x000000);
    }

    FastLED.addLeds<WS2812, DATA_PIN, EOrder::RGB>(CURRENT_COLORS, NUM_LEDS);

    for(size_t i = 0; i < DEPLOYED_VERSION; ++i)
    {
        CURRENT_COLORS[0] = CRGB(0xffffff);
        FastLED.show();
        delay(500);
        CURRENT_COLORS[0] = CRGB(0x000000);
        FastLED.show();
        delay(500);
    }

    uint8_t macAddress[8] = {0};
    esp_efuse_mac_get_default(macAddress);
    unsigned long seed = 0;
    for(size_t i = 0; i < sizeof(macAddress) / sizeof(unsigned long); ++i)
    {
        seed += reinterpret_cast<unsigned long*>(macAddress)[i];
    }
    randomSeed(seed);

    double minFrequency = -1.0;
    for(size_t i = 0; i < NUM_ORBITS; ++i)
    {
        ORBIT_FREQUENCIES[i] = 2.0 * (random() / static_cast<double>(RAND_MAX)) + 0.1;
        if(minFrequency < 0.0 || ORBIT_FREQUENCIES[i] < minFrequency)
        {
            minFrequency = ORBIT_FREQUENCIES[i];
        }
    }
    HUE_FREQUENCY = 0.02 * (random() / static_cast<double>(RAND_MAX)) + 0.005;

    double minDuration = 2.0 / minFrequency; // , baseTime = 0.0;
    for(size_t i = 0; i < NUM_ANIMATIONS; ++i)
    {
        ANIM_LENGTHS[i] = minDuration + 5.0 * (random() / static_cast<double>(RAND_MAX));
        // baseTime = ANIM_END_TIMES[i];
    }

    PREVIOUS_MILLIS = millis();
}

void loop()
{
    unsigned long newMillis = millis();
    double delta = (newMillis - PREVIOUS_MILLIS) / 1000.0;
    for(size_t i = 0; i < NUM_ORBITS; ++i)
    {
        ORBIT_PHASES[i] += ORBIT_FREQUENCIES[i] * delta;
        ORBIT_PHASES[i] -= static_cast<int>(ORBIT_PHASES[i]);
    }
    // PHASE_1 += frequency_1 * delta; PHASE_1 -= static_cast<int>(PHASE);
    // PHASE_2 += frequency_2 * delta; PHASE_2 -= static_cast<int>(PHASE);
    HUE_PHASE += HUE_FREQUENCY * delta; HUE_PHASE -= static_cast<int>(HUE_PHASE);

    CURRENT_ANIM_TIME += delta;
    while(CURRENT_ANIM_TIME >= ANIM_LENGTHS[CURRENT_ANIMATION])
    {
        CURRENT_ANIM_TIME -= ANIM_LENGTHS[CURRENT_ANIMATION];
        CURRENT_ANIMATION = (CURRENT_ANIMATION + 1) % NUM_ANIMATIONS;
    }

    PREVIOUS_MILLIS = newMillis;

    // size_t currentAnimation = (newMillis % (5000 * NUM_ANIMATIONS)) / 5000;

    for(size_t orbit = 0; orbit < NUM_ORBITS; ++orbit)
    {
        for(size_t led = 0; led < ORBIT_SIZE; ++led)
        {
            double adjustedPhase = ORBIT_PHASES[orbit] + static_cast<double>(led) / ORBIT_SIZE, thisHuePhase = HUE_PHASE;
            adjustedPhase -= static_cast<int>(adjustedPhase);
            double brightness = getBrightness(orbit, adjustedPhase, CURRENT_ANIM_TIME, ANIM_LENGTHS[CURRENT_ANIMATION]);

            if(orbit == 1)
            {
                thisHuePhase += 0.5;
                thisHuePhase -= static_cast<int>(thisHuePhase);
            }
            // CHSV color = CHSV(static_cast<uint8_t>((orbit == 0 ? HUE_PHASE : (1.0 - HUE_PHASE)) * 255), 255, static_cast<uint8_t>(brightness * 255));
            // CRGB color = CRGB(0x000000);
            // (orbit == 0 ? color.red : (currentAnimation == 0 ? color.blue : color.green)) = static_cast<uint8_t>(brightness * 255);
            CURRENT_COLORS[ORBITS[CURRENT_ANIMATION][orbit][led]].setHSV(static_cast<uint8_t>(thisHuePhase * 255), 255, static_cast<uint8_t>(brightness * 255));
        }
    }
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