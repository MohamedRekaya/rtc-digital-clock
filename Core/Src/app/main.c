/**
  ******************************************************************************
  * @file    main.c
  * @brief   Test program using proper RTC hardware flags
  ******************************************************************************
  */
#include "stm32f4xx.h"
#include "rtc.h"
#include "led.h"
#include "systick.h"

int main(void) {
    systick_init();
    led_init();

    /* Startup blink */
    led_all_on();
    systick_delay_ms(300);
    led_all_off();
    systick_delay_ms(300);

    /* Check RTC status before initialization */
    rtc_status_t status = rtc_get_status();

    /* Show status on LEDs */
    switch(status) {
        case RTC_STATUS_NOT_INITIALIZED:
            led_on(LED_RED);
            break;
        case RTC_STATUS_INIT_MODE:
            led_on(LED_ORANGE);
            break;
        case RTC_STATUS_RUNNING:
            led_on(LED_GREEN);
            break;
        default:
            led_on(LED_BLUE);
            break;
    }
    systick_delay_ms(1000);
    led_all_off();

    /* Initialize RTC (only if not already initialized) */
    if (!rtc_init()) {
        /* RTC init failed - blink red fast */
        while(1) {
            led_on(LED_RED);
            systick_delay_ms(100);
            led_off(LED_RED);
            systick_delay_ms(100);
        }
    }

    /* RTC initialized - blink green 3 times */
    for(int i = 0; i < 3; i++) {
        led_on(LED_GREEN);
        systick_delay_ms(200);
        led_off(LED_GREEN);
        systick_delay_ms(200);
    }

    /* Set time if not already set */
    rtc_time_t time;
    rtc_get_time(&time);

    /* If time is 00:00:00, set it to 12:00:00 */
    if(time.hours == 0 && time.minutes == 0 && time.seconds == 0) {
        time.hours = 12;
        time.minutes = 0;
        time.seconds = 0;
        rtc_set_time(&time);

        /* Flash blue to show time set */
        led_on(LED_BLUE);
        systick_delay_ms(300);
        led_off(LED_BLUE);
    }

    /* Main loop - show seconds on LEDs */
    uint8_t last_second = 0xFF;

    while(1) {
        rtc_get_time(&time);

        if(time.seconds != last_second) {
            last_second = time.seconds;

            /* Toggle green LED every second */
            led_toggle(LED_GREEN);

            /* Show seconds in binary on other LEDs */
            led_off(LED_RED | LED_ORANGE | LED_BLUE);

            if(time.seconds & 0x01) led_on(LED_RED);
            if(time.seconds & 0x02) led_on(LED_ORANGE);
            if(time.seconds & 0x04) led_on(LED_BLUE);
        }

        systick_delay_ms(10);
    }
}
