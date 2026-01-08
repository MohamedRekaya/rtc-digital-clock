#include "led.h"
#include "board_config.h"
#include "stm32f4xx.h"
#include "systick.h"

// Convert LED ID to GPIO pin
static uint16_t led_id_to_pin(led_id_t led) {
    switch (led) {
        case LED_GREEN:  return LED_GREEN_PIN_MSK;
        case LED_ORANGE: return LED_ORANGE_PIN_MSK;
        case LED_RED:    return LED_RED_PIN_MSK;
        case LED_BLUE:   return LED_BLUE_PIN_MSK;
        default:         return 0;
    }
}



// Static variables
static led_blink_ctrl_t blink_ctrl[LED_COUNT] = {0};







void led_init(void) {
	// 1. Enable GPIO clock
	LED_GPIO_CLK_ENABLE();

	// 2. Configure different LED pins
	uint32_t pins[] = {LED_GREEN_PIN_NUM, LED_ORANGE_PIN_NUM, LED_RED_PIN_NUM, LED_BLUE_PIN_NUM};

	for (int i = 0; i < 4; i++){
		uint32_t pin = pins[i];

		// MODER: Output mode
		GPIOD -> MODER &= ~(3U << (pin * 2));
		GPIOD -> MODER |= (1U << (pin *2));

		// OTYPE: Push-Pull
		GPIOD -> OTYPER &= ~(pin);

		// OSPEED: Medium speed
		GPIOD -> OSPEEDR &= ~(3U << (pin * 2));
		GPIOD -> OSPEEDR |= (2U << (pin * 2));

		// PUPDR: No pull
		GPIOD -> PUPDR &= ~(3U << (pin * 2));
	}


	// 3. Initial state: OFF
	led_all_off();
}

void led_on(led_id_t led) {
    if (led == LED_ALL) {
        led_all_on();
        return;
    }

    if (led < LED_COUNT) {
        uint16_t pin = led_id_to_pin(led);
        LED_GPIO_PORT->BSRR = pin;  // Set pin (turn ON)
    }
}

void led_off(led_id_t led) {
    if (led == LED_ALL) {
        led_all_off();
        return;
    }

    if (led < LED_COUNT) {
        uint16_t pin = led_id_to_pin(led);
        LED_GPIO_PORT->BSRR = (uint32_t)pin << 16;  // Reset pin (turn OFF)
    }
}

void led_toggle(led_id_t led) {
    if (led == LED_ALL) {
        led_all_toggle();
        return;
    }

    if (led < LED_COUNT) {
        uint16_t pin = led_id_to_pin(led);
        LED_GPIO_PORT->ODR ^= pin;  // Toggle using XOR
    }
}

bool led_is_on(led_id_t led) {
    if (led < LED_COUNT) {
        uint16_t pin = led_id_to_pin(led);
        return (LED_GPIO_PORT->ODR & pin) != 0;
    }
    return false;
}

void led_set(led_id_t led, bool state) {
    if (state) {
        led_on(led);
    } else {
        led_off(led);
    }
}

void led_all_on(void) {
    LED_GPIO_PORT->BSRR = LED_ALL_PINS;
}

void led_all_off(void) {
    LED_GPIO_PORT->BSRR = (uint32_t)LED_ALL_PINS << 16;
}

void led_all_toggle(void) {
    LED_GPIO_PORT->ODR ^= LED_ALL_PINS;
}

void led_set_pattern(uint8_t pattern) {
    // Clear all LEDs first
    led_all_off();

    // Set LEDs based on pattern bits
    if (pattern & 0x01) led_on(LED_GREEN);   // Bit 0: Green
    if (pattern & 0x02) led_on(LED_ORANGE);  // Bit 1: Orange
    if (pattern & 0x04) led_on(LED_RED);     // Bit 2: Red
    if (pattern & 0x08) led_on(LED_BLUE);    // Bit 3: Blue
}

// Simple blocking delay (temporary - will be replaced by systick)
static void delay_ms_blocking(uint32_t ms) {
    for (uint32_t i = 0; i < ms * 1600; i++) {
        __NOP();
    }
}

void led_chase(uint32_t delay_ms) {
    static uint8_t chase_state = 0;

    led_all_off();

    switch (chase_state) {
        case 0: led_on(LED_GREEN); break;
        case 1: led_on(LED_ORANGE); break;
        case 2: led_on(LED_RED); break;
        case 3: led_on(LED_BLUE); break;
    }

    chase_state = (chase_state + 1) % 4;
    delay_ms_blocking(delay_ms);
}

void led_knight_rider(void) {
    static int8_t direction = 1;
    static uint8_t position = 0;

    led_all_off();

    // Light current position
    led_set((led_id_t)position, true);

    // Update position
    position += direction;

    // Reverse direction at ends
    if (position == 0 || position == LED_COUNT - 1) {
        direction = -direction;
    }

    delay_ms_blocking(200);
}

// SIMPLE Blink - uses counter instead of time
void led_blink(led_id_t led, uint32_t on_time_ms, uint32_t off_time_ms) {
	if (led >= LED_COUNT || on_time_ms == 0 || off_time_ms == 0) return;

    blink_ctrl[led].on_time_ms = on_time_ms;
    blink_ctrl[led].off_time_ms = off_time_ms;
    blink_ctrl[led].last_toggle_ms = systick_get_ticks();  // Use systick!
    blink_ctrl[led].is_on = true;
    blink_ctrl[led].is_blinking = true;

    led_on(led);
}

void led_blink_stop(led_id_t led) {
    if (led >= LED_COUNT) return;
    blink_ctrl[led].is_blinking = false;
    led_off(led);
}


void led_update_all(void) {
    uint32_t current_time = systick_get_ticks();  // Get current time

    for (led_id_t led = LED_GREEN; led < LED_COUNT; led++) {
        if (!blink_ctrl[led].is_blinking) continue;

        uint32_t elapsed = current_time - blink_ctrl[led].last_toggle_ms;

        if (blink_ctrl[led].is_on) {
            // Currently ON, check if time to turn OFF
            if (elapsed >= blink_ctrl[led].on_time_ms) {
                led_off(led);
                blink_ctrl[led].is_on = false;
                blink_ctrl[led].last_toggle_ms = current_time;
            }
        } else {
            // Currently OFF, check if time to turn ON
            if (elapsed >= blink_ctrl[led].off_time_ms) {
                led_on(led);
                blink_ctrl[led].is_on = true;
                blink_ctrl[led].last_toggle_ms = current_time;
            }
        }
    }
}

























