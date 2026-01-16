/**
  ******************************************************************************
  * @file    main.c
  * @brief   RTC with LCD display and bell icon
  ******************************************************************************
  */

#include "stm32f4xx.h"
#include "rtc.h"
#include "systick.h"
#include "i2c.h"
#include "lcd1602_i2c.h"
#include "custom_chars.h"

static rtc_time_t time;
static rtc_date_t date;

/* Weekday names */
const char* weekdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

/* Alarm state - always show bell in this version */
static bool show_bell = true;

/**
  * @brief  RTC periodic callback
  */
void rtc_periodic_callback(void) {
    rtc_get_time(&time);
    rtc_get_date(&date);
}

int main(void) {
    /* Initialize */
    systick_init();
    i2c_init();
    lcd_init();

    /* Initialize bell character (location 0) */
    lcd_create_char(0, bell_char);

    /* Initialize RTC */
    rtc_init();
    rtc_periodic_init(RTC_PERIODIC_EVERY_SECOND);

    /* Enable interrupts */
    __enable_irq();

    lcd_clear();

    while (1) {
        /* Line 1: Time with bell icon */
        lcd_set_cursor(0, 4);  /* Center time (8 chars total) */

        /* Hours */
        lcd_write_char('0' + (time.hours / 10));
        lcd_write_char('0' + (time.hours % 10));
        lcd_write_char(':');

        /* Minutes */
        lcd_write_char('0' + (time.minutes / 10));
        lcd_write_char('0' + (time.minutes % 10));
        lcd_write_char(':');

        /* Seconds */
        lcd_write_char('0' + (time.seconds / 10));
        lcd_write_char('0' + (time.seconds % 10));

        /* Bell icon right after time */
        lcd_set_cursor(0, 13);  /* After time (HH:MM:SS = 8 chars, 4+8=12) */
        if (show_bell) {
            lcd_write_custom_char(0);  /* Bell icon */
        } else {
            lcd_write_char(' ');  /* Space if bell hidden */
        }

        /* Line 2: Date with weekday */
        lcd_set_cursor(1, 1);

        /* Show weekday (0-6) */
        if (date.weekday <= 6) {
            lcd_write_string(weekdays[date.weekday]);
        } else {
            lcd_write_string("---");
        }
        lcd_write_char(' ');

        /* Show date DD/MM/YYYY */
        lcd_write_char('0' + (date.day / 10));
        lcd_write_char('0' + (date.day % 10));
        lcd_write_char('/');
        lcd_write_char('0' + (date.month / 10));
        lcd_write_char('0' + (date.month % 10));
        lcd_write_char('/');
        lcd_write_char('2');
        lcd_write_char('0');
        lcd_write_char('0' + (date.year % 100) / 10);
        lcd_write_char('0' + (date.year % 100) % 10);

        systick_delay_ms(100);
    }
}
