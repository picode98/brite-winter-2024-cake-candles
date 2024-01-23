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

const int DATA_PIN = 12;
const int ORBITS[][2][4] = {
    {
        {2, 3, 4, 6},
        {0, 7, 5, 1}
    },
    {
        {1, 3, 4, 5},
        {7, 0, 2, 6}
    }
};
const size_t NUM_ORBITS = sizeof(ORBITS[0]) / sizeof(ORBITS[0][0]), ORBIT_SIZE = sizeof(ORBITS[0][0]) / sizeof(ORBITS[0][0][0]);

const size_t NUM_LEDS = 8;
CRGB CURRENT_COLORS[NUM_LEDS];
// size_t CURRENT_ANIMATION = 0;
double PHASE = 0.0, HUE_PHASE = 0.0;
unsigned long PREVIOUS_MILLIS;

double getBrightness(double phase)
{
    return max(1.0 - 4.0 * phase, 0.0);
}

void setup()
{
    FastLED.addLeds<WS2812, DATA_PIN, EOrder::RGB>(CURRENT_COLORS, NUM_LEDS);
    PREVIOUS_MILLIS = millis();
}

void loop()
{
    double frequency = 1.0, hueFrequency = 0.01;
    unsigned long newMillis = millis();
    PHASE += frequency * (newMillis - PREVIOUS_MILLIS) / 1000.0;
    PHASE -= static_cast<int>(PHASE);
    HUE_PHASE += hueFrequency * (newMillis - PREVIOUS_MILLIS) / 1000.0;
    HUE_PHASE -= static_cast<int>(HUE_PHASE);
    PREVIOUS_MILLIS = newMillis;

    size_t currentAnimation = (newMillis % 10000 >= 5000);

    for(size_t orbit = 0; orbit < NUM_ORBITS; ++orbit)
    {
        for(size_t led = 0; led < ORBIT_SIZE; ++led)
        {
            double adjustedPhase = PHASE + static_cast<double>(led) / ORBIT_SIZE;
            adjustedPhase -= static_cast<int>(adjustedPhase);
            double brightness = getBrightness(adjustedPhase);
            // CHSV color = CHSV(static_cast<uint8_t>((orbit == 0 ? HUE_PHASE : (1.0 - HUE_PHASE)) * 255), 255, static_cast<uint8_t>(brightness * 255));
            // CRGB color = CRGB(0x000000);
            // (orbit == 0 ? color.red : (currentAnimation == 0 ? color.blue : color.green)) = static_cast<uint8_t>(brightness * 255);
            CURRENT_COLORS[ORBITS[currentAnimation][orbit][led]].setHSV(static_cast<uint8_t>((orbit == 0 ? HUE_PHASE : (1.0 - HUE_PHASE)) * 255), 255, static_cast<uint8_t>(brightness * 255));
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