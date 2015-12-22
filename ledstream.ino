#include <FastLED.h>

#define NUM_LEDS 150
#define DATA_PIN 3
#define SERIALRATE 115200
#define CALIBRATION_TEMPERATURE TypicalLEDStrip  // Color correction
#define MAX_BRIGHTNESS 255 // 0-255

// Before sending the pixel data, Adalight sends a magic word,
// the pixel count (as a high and a low byte), and a checksum.

static const uint8_t MAGIC[] = {'A', 'd', 'a'};

CRGB leds[NUM_LEDS];

void setup() {
	FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

	FastLED.setTemperature(CALIBRATION_TEMPERATURE);
	FastLED.setBrightness(MAX_BRIGHTNESS);

	// Issue test pattern to LEDs on startup.
	CRGB test_colors[] = { CRGB::Magenta, CRGB::Cyan, CRGB::Yellow, CRGB::Black };
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

void loop() {
	uint8_t count_hi, count_lo, checksum, i;

	// wait for first byte of Magic Word
	for(i = 0; i < sizeof(MAGIC); ++i) {
waitForMagicLoop:
		while (!Serial.available());

		if (MAGIC[i] == Serial.read()) {
			continue;
		}

		i = 0;

		goto waitForMagicLoop;
	}

	while (!Serial.available());
	count_hi = Serial.read();

	while (!Serial.available());
	count_lo = Serial.read();

	while (!Serial.available());
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
