#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <Keypad.h>
#include <MM545x.h>

#define MM5450_CLOCK_PIN	10
#define MM5450_DATA_PIN		9

#define KEYPAD_LED_PIN		3
#define KEYPAD_LED_COUNT	5
#define KEYPAD_ROWS		4
#define KEYPAD_COLS		3

char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'B','3','7'},
  {'1','5','9'},
  {'_','4','8'},
  {'A','2','6'}
};
byte rowPins[KEYPAD_ROWS] = {6, 7, 8, A1};
byte colPins[KEYPAD_COLS] = {A3, A2, 4}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS );
Adafruit_NeoPixel keyboard_leds = Adafruit_NeoPixel(KEYPAD_LED_COUNT, KEYPAD_LED_PIN, NEO_GRB + NEO_KHZ800);
MM545x mm5450(MM5450_CLOCK_PIN, MM5450_DATA_PIN);

void setup() {
  keyboard_leds.begin();
  keyboard_leds.show(); // Initialize all pixels to 'off'
  Serial.begin(9600);
}

void loop() {
	uint32_t color;
	char key = keypad.getKey();	  
	if (key){
		if (key >= '1' && key <= '9') {
			color = Wheel(75 + (key - '1') * 20);
		} else if (key == 'A') {
			color = Wheel(0);
		} else {
			for (int i = 0; i < 34; i++) {
				mm5450.setLeds((uint64_t) 1 << i);
				delay(100);
			}
			color = Wheel(37);
		}
		for (int i = 0; i < KEYPAD_LED_COUNT; i++) {
			keyboard_leds.setPixelColor(i, color);
		}
		Serial.println(key);
		keyboard_leds.show();
	}
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return keyboard_leds.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return keyboard_leds.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return keyboard_leds.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
