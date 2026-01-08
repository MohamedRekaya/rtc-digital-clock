

#ifndef LED_H
#define LED_H

#include <stdbool.h>
#include <stdint.h>
#include "board_config.h"



// Remove all timing structures - keep it simple!
typedef struct {
    uint32_t last_toggle_ms; // Last toggle time (from systick)
    uint32_t on_time_ms;
    uint32_t off_time_ms;
    bool is_on;
    bool is_blinking;
} led_blink_ctrl_t;

//================================
// Basic LED functions
//================================

/**
 * @brief Initialize LED GPIO pin
 * @note Must enable GPIO clock before calling
 */
void led_init(void);

/**
 * @brief Turn specific LED ON
 * @param led LED identifier (LED_GREEN, LED_ORANGE, etc.)
 */
void led_on(led_id_t led);

/**
 * @brief Turn specific LED OFF
 * @param led LED identifier
 */
void led_off(led_id_t led);

/**
 * @brief Toggle specific LED
 * @param led LED identifier
 */
void led_toggle(led_id_t led);

/**
 * @brief Set specific LED state
 * @param led LED identifier
 * @param state true = ON, false = OFF
 */
void led_set(led_id_t led, bool state);

/**
 * @brief Get LED state
 * @param led LED identifier
 * @return true if ON, false if OFF
 */
bool led_is_on(led_id_t led);

//=============================
// Multi-LED functions
//=============================

/**
 * @brief Turn all LEDs ON
 */
void led_all_on(void);

/**
 * @brief Turn all LEDs OFF
 */
void led_all_off(void);

/**
 * @brief Toggle all LEDs
 */
void led_all_toggle(void);

//============================
// Pattern functions
//===========================

/**
 * @brief Set all LEDs to specific pattern
 * @param pattern 4-bit pattern (bit0: GREEN, bit1: ORANGE, bit2: RED, bit3: BLUE)
 */
void led_set_pattern(uint8_t pattern);

/**
 * @brief Simple chase pattern (for testing)
 * @param delay_ms Time between steps (ms)
 */
void led_chase(uint32_t delay_ms);

/**
 * @brief Knight Rider pattern
 */
void led_knight_rider(void);

//=============================
// Blink functions
//============================


/**
 * @brief Configure specific LED to blink automatically
 * @param led LED identifier (LED_GREEN, LED_ORANGE, etc.)
 * @param on_time_ms Time LED stays ON (milliseconds)
 * @param off_time_ms Time LED stays OFF (milliseconds)
 */
void led_blink(led_id_t led, uint32_t on_threshold, uint32_t off_threshold) ;

/**
 * @brief Stop blinking for specific LED
 * @param led LED identifier
 */
void led_blink_stop(led_id_t led);

/**
 * @brief Update all blinking LEDs (call in main loop)
 */
void led_update_all(void);












#endif /* LED_H */
