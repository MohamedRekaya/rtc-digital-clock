#include "stm32f4xx.h"
#include "rtc.h"
#include "led.h"
#include "systick.h"

/* Override alarm callback */
void rtc_alarm_callback(void) {
    /* This is called when alarm triggers */
    led_all_on();
    systick_delay_ms(500);
    led_all_off();
}

int main(void) {
    /* Initialize system */
    systick_init();
    led_init();

    /* Initialize RTC */
    if (!rtc_init()) {
        /* Error handling */
        while(1);
    }

    /* Initialize alarm system (once) */
    rtc_alarm_init();

    /* Set current time (example: 12:00:00) */
    rtc_time_t time = {12, 0, 0};
    rtc_set_time(&time);

    /* Set alarm for 12:00:30 (30 seconds from now) */
    rtc_alarm_t alarm = {
        .hour = 12,
        .minute = 0,
        .second = 10,
        .mask = RTC_ALARM_MASK_HH_MM_SS,  /* Match hour, minute, second */
        .weekday = 0,                      /* Daily alarm (no weekday match) */
        .enabled = true
    };

    if (!rtc_set_alarm_a(&alarm)) {
        /* Alarm setup failed */
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Main loop */
    while (1) {
        /* Normal operation */
        rtc_time_t current_time;
        rtc_get_time(&current_time);

        /* Show seconds on LEDs */
        led_off(LED_RED | LED_ORANGE | LED_BLUE);
        if (current_time.seconds & 0x01) led_on(LED_RED);
        if (current_time.seconds & 0x02) led_on(LED_ORANGE);
        if (current_time.seconds & 0x04) led_on(LED_BLUE);

        systick_delay_ms(100);
    }
}
