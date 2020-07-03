#include <FastLED.h>

// WS2812b
#define NUM_LEDS 48
#define DATA_PIN 3

// 50 LED WS2801
// #define NUM_LEDS 50

// #define NUM_LEDS 32

#define SERIALRATE 115200
#define CALIBRATION_TEMPERATURE TypicalLEDStrip  // Color correction
#define MAX_BRIGHTNESS 255 // 0-255

// Before sending the pixel data, Adalight sends a magic word,
// the pixel count (as a high and a low byte), and a checksum.

static const uint8_t MAGIC[] = {'A', 'd', 'a'};

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  // FastLED.addLeds<WS2801, RGB>(leds, NUM_LEDS);

  FastLED.setTemperature(CALIBRATION_TEMPERATURE);
  FastLED.setBrightness(MAX_BRIGHTNESS);

  // Issue test pattern to LEDs on startup.
  CRGB test_colors[] = { CRGB::Red, CRGB::Cyan, CRGB::Yellow, CRGB::Black };
  uint16_t i;
  for (i = 0; i < (sizeof(test_colors) / sizeof(CRGB)); i++) {
    uint16_t j;
    for (j = 0; j < NUM_LEDS; j++) {
      leds[j] = test_colors[i];
    }

    FastLED.show();
    delay(800);
  }

  Serial.begin(SERIALRATE);
  Serial.print("Ada\n"); // Send the magic word to the host
}

const uint64_t N_BEFORE_EXTINGUISH = 100000;

void waitForAvailable() {
  for (uint64_t i = N_BEFORE_EXTINGUISH; i > 0; i--) {
    if (Serial.available()) return;
  }
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  while (!Serial.available());
}

void loop() {
  uint8_t count_hi, count_lo, checksum;
  uint16_t i;

  // wait for first byte of Magic Word
  for (i = 0; i < sizeof(MAGIC); ++i) {
  waitForMagicLoop:
    waitForAvailable();

    if (MAGIC[i] == Serial.read()) {
      continue;
    }

    i = 0;

    goto waitForMagicLoop;
  }

  waitForAvailable();
  count_hi = Serial.read();

  waitForAvailable();
  count_lo = Serial.read();

  waitForAvailable();
  checksum = Serial.read();

  // If the checksum does not match, go back to waitForMagicLoop
  if (checksum != (count_hi ^ count_lo ^ 0x55)) {
    i = 0;
    goto waitForMagicLoop;
  }

  Serial.readBytes((uint8_t*)leds, NUM_LEDS * 3);

  // Light up the LEDs using the data in `leds`
  FastLED.show();
}
