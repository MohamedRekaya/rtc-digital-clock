#include "stm32f4xx.h"
#include "rtc.h"
#include "led.h"
#include "systick.h"

/* Simple counter to verify interrupt frequency */
volatile uint32_t interrupt_counter = 0;

/* Very simple periodic callback */
void rtc_periodic_callback(void) {
    interrupt_counter++;  /* Count interrupts */
    led_toggle(LED_ORANGE);  /* Visual proof - toggle LED every second */
}

int main(void) {
    /* Initialize minimum required */
    systick_init();
    led_init();

    /* Startup indicator */
    led_on(LED_RED);
    systick_delay_ms(100);
    led_off(LED_RED);

    /* Initialize RTC */
    if (!rtc_init()) {
        /* Failed - blink red fast */
        while(1) {
            led_toggle(LED_RED);
            systick_delay_ms(100);
        }
    }

    /* Set initial time */
    rtc_time_t time = {0, 0, 0};
    rtc_set_time(&time);

    /* Initialize periodic interrupt for 1 second */
    if (!rtc_periodic_init(RTC_PERIODIC_EVERY_SECOND)) {
        /* Failed - blink orange */
        while(1) {
            led_toggle(LED_ORANGE);
            systick_delay_ms(200);
        }
    }

    /* Enable interrupts */
    __enable_irq();

    /* Turn on blue LED to show system is running */
    led_on(LED_BLUE);

    uint32_t last_counter = 0;
    uint32_t last_time = 0;

    while (1) {
        /* Get current RTC time */
        rtc_time_t current_time;
        rtc_get_time(&current_time);

        /* Every 5 seconds, verify interrupt frequency */
        if (current_time.seconds >= last_time + 5) {
            last_time = current_time.seconds;

            /* Calculate interrupts per 5 seconds */
            uint32_t interrupts_in_5s = interrupt_counter - last_counter;
            last_counter = interrupt_counter;

            /* Visual verification:
               - If we got exactly 5 interrupts in 5 seconds -> blink green once
               - If we got more/less -> blink red based on error count */
            if (interrupts_in_5s == 5) {
                /* Perfect timing - quick green flash */
                led_on(LED_BLUE);
                systick_delay_ms(200);
                led_off(LED_BLUE);
            } else {
                /* Error - blink red based on error count */
                uint8_t error_count = (interrupts_in_5s > 5) ?
                                      (interrupts_in_5s - 5) :
                                      (5 - interrupts_in_5s);

                for (uint8_t i = 0; i < error_count; i++) {
                    led_on(LED_RED);
                    systick_delay_ms(200);
                    led_off(LED_RED);
                    systick_delay_ms(200);
                }
            }
        }

        /* Simple delay - minimal CPU usage */
        systick_delay_ms(10);
    }
}
