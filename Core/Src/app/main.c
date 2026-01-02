#include "stm32f4xx.h"
#include "led.h"
#include "button.h"
#include "systick.h"
#include "pattern_manager.h"
#include "sleep_manager.h"

int main(void) {
    /* 1. Initialize system (ORDER MATTERS!) */
    systick_init();             /* Must be first for timing */
    led_init();                 /* Initialize LEDs */
    button_init();              /* Initialize button with EXTI */
    pattern_manager_init();     /* Initialize pattern manager */
    sleep_manager_init();       /* Initialize sleep manager */

    /* 2. Enable global interrupts */
    __enable_irq();

    /* 3. Startup animation */
    for (int i = 0; i < 3; i++) {
        led_all_on();
        for (int j = 0; j < 200000; j++);
        led_all_off();
        for (int j = 0; j < 200000; j++);
    }

    /* 4. Start with first pattern */
    pattern_manager_set_pattern(PATTERN_SOLID);
    pattern_manager_start();

    /* 5. Main superloop */
    while (1) {
        /* Update button state machine */
        button_update();

        /* Check for wakeup request */
        static bool was_sleeping = false;
        if (sleep_manager_is_sleeping() && !was_sleeping) {
            // System just entered sleep
            was_sleeping = true;

            // Sleep indication (slow blink while sleeping)
            while (sleep_manager_is_sleeping()) {
                // Slow blink Green LED (1Hz) to show sleep state
                static uint32_t sleep_blink_timer = 0;
                if (systick_delay_elapsed(sleep_blink_timer, 500)) {
                    led_toggle(LED_GREEN);
                    sleep_blink_timer = systick_get_ticks();
                }

                // Check for wakeup
                button_update();
            }

            was_sleeping = false;
            continue;  // Restart loop after wakeup
        }

        /* Handle button events (only when awake) */
        button_event_t event = button_get_event();

        switch (event) {
            case BUTTON_EVENT_PRESSED:
                /* Short press: Toggle pattern pause/resume */
                if (pattern_manager_get_state() == PATTERN_STATE_RUNNING) {
                    pattern_manager_pause();
                    led_set_pattern(0b1010);  // Show paused state
                } else {
                    pattern_manager_resume();
                }
                break;

            case BUTTON_EVENT_LONG_PRESS:
                /* Long press: Next pattern */
                pattern_manager_next();

                /* Visual feedback */
                led_all_on();
                for (volatile int i = 0; i < 20000; i++);
                led_all_off();
                break;

            case BUTTON_EVENT_DOUBLE_CLICK:
                /* Double click: Enter sleep mode */
                sleep_manager_enter();
                break;

            case BUTTON_EVENT_RELEASED:
                /* Release events can be used for additional features */
                break;

            default:
                /* No event */
                break;
        }

        /* Update pattern (when not sleeping) */
        if (!sleep_manager_is_sleeping()) {
            pattern_manager_update();
        }

        /* Small delay to reduce CPU usage */
        for (volatile int i = 0; i < 1000; i++);
    }
}
