#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#include <Keypad.h>
#include <MM545x.h>
#include <Servo.h>

/* M5450 related */
#define MM5450_CLOCK_PIN	10
#define MM5450_DATA_PIN		9

/* Keypad related */
#define KEYPAD_LED_PIN		3
#define KEYPAD_LED_COUNT	5
#define KEYPAD_ROWS		4
#define KEYPAD_COLS		3

#define SERVO_PIN		A0
#define SERVO_START		20
#define SERVO_END		120

char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'B', '3', '7'},
  {'1', '5', '9'},
  {'_', '4', '8'},
  {'A', '2', '6'}
};
byte rowPins[KEYPAD_ROWS] = {6, 7, 8, A1};
byte colPins[KEYPAD_COLS] = {A3, A2, 4}; //connect to the column pinouts of the keypad

/* Keypad related */
#define LED_RING_PIN      2
#define LED_RING_COUNT    12
#define LED_PATTERN_RING_COUNT   10

/* Objects */
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS );
Adafruit_NeoPixel keyboard_leds = Adafruit_NeoPixel(KEYPAD_LED_COUNT, KEYPAD_LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel led_ring = Adafruit_NeoPixel(LED_RING_COUNT, LED_RING_PIN, NEO_GRB + NEO_KHZ800);
MM545x mm5450(MM5450_CLOCK_PIN, MM5450_DATA_PIN);
Servo servo;

#define LED_RING_STATE_OFF  0
#define LED_RING_STATE_LOADING  1
#define LED_RING_STATE_BEATING  2


unsigned long led_ring_state = LED_RING_STATE_OFF;

unsigned long long ledring_last_time = 0;
void setup() {
	keyboard_leds.begin();
	keyboard_leds.setBrightness(80);
	keyboard_leds.show();
	led_ring.begin();
	led_ring.setBrightness(80);
	led_ring.show();
	Serial.begin(9600);
	servo.attach(SERVO_PIN);
	servo.write(SERVO_START);
	delay(400);
	servo.detach();
}

unsigned int current_led = 0;
unsigned int current_counter = 0;
double fading = 0;

unsigned long long wait_time = 100000;

static void led_ring_handler()
{
	if (led_ring_state == LED_RING_STATE_LOADING) {
		if (micros() - ledring_last_time < wait_time)
			return;

		ledring_last_time = micros();
		led_ring.clear();
		for (int i = 0; i < LED_PATTERN_RING_COUNT; i++) {
			led_ring.setPixelColor((current_led + i) % LED_RING_COUNT, led_ring.Color(0, (255 / LED_PATTERN_RING_COUNT) * i , (255 / LED_PATTERN_RING_COUNT) * i));
		}
		led_ring.show();
		current_led++;
		current_led %= LED_RING_COUNT;
	}
}

static int cur_servo_pos = SERVO_START;
static int req_servo_pos = SERVO_START;
static unsigned long long servo_last_time = 0;

static void servo_handler()
{
	if (req_servo_pos == cur_servo_pos)
		return;
	if (micros() - servo_last_time < 10000)
		return;

	servo_last_time = micros();

	if (req_servo_pos < cur_servo_pos)
		servo.write(cur_servo_pos--);
	else
		servo.write(cur_servo_pos++);

	if (req_servo_pos == cur_servo_pos)
		servo.detach();

}
void ledring_set_state(int state)
{
	led_ring_state = state;
	led_ring.clear();
	led_ring.show();
}

void servo_set(int value)
{
	req_servo_pos = value;
	servo.attach(SERVO_PIN);
}

uint32_t Wheel(byte WheelPos) {
	WheelPos = 255 - WheelPos;
	if (WheelPos < 85) {
		return keyboard_leds.Color(255 - WheelPos * 3, 0, WheelPos * 3);
	}
	if (WheelPos < 170) {
		WheelPos -= 85;
		return keyboard_leds.Color(0, WheelPos * 3, 255 - WheelPos * 3);
	}
	WheelPos -= 170;
	return keyboard_leds.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void loop() {
	uint8_t digit;
	uint32_t color;
	char key = keypad.getKey();
	if (key) {
		if (key >= '1' && key <= '9') {
			digit = (key - '1');
			color = Wheel(75 + digit * 20);

		} else if (key == 'A') {
			color = Wheel(0);
			ledring_set_state(LED_RING_STATE_LOADING);
		} else {
			color = Wheel(37);
			ledring_set_state(LED_RING_STATE_BEATING);
		}
		switch (key) {
			case '1':
				wait_time += 10000;
			break;
			case '2':
				wait_time -= 10000;
			break;
			case '4':
				servo_set(SERVO_START);
			break;
			case '5':
				servo_set(SERVO_END);
			break;
			default:
			break;
		}
		for (int i = 0; i < KEYPAD_LED_COUNT; i++) {
			keyboard_leds.setPixelColor(i, color);
		}

		keyboard_leds.show();
		Serial.println(key);
	}
	led_ring_handler();
	servo_handler();
}

