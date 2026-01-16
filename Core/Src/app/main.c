#include "stm32f4xx.h"
#include "rtc.h"
#include "systick.h"
#include "i2c.h"
#include "lcd1602_i2c.h"

static rtc_time_t time;
static rtc_date_t date;

/* Weekday names */
const char* weekdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

void rtc_periodic_callback(void) {
    rtc_get_time(&time);
    rtc_get_date(&date);
}

int main(void) {
    systick_init();
    i2c_init();
    lcd_init();
    rtc_init();
    rtc_periodic_init(RTC_PERIODIC_EVERY_SECOND);
    __enable_irq();

    lcd_clear();

    while (1) {
        /* Line 1: Time */
        lcd_set_cursor(0, 0);
        lcd_write_char('0' + (time.hours / 10));
        lcd_write_char('0' + (time.hours % 10));
        lcd_write_char(':');
        lcd_write_char('0' + (time.minutes / 10));
        lcd_write_char('0' + (time.minutes % 10));
        lcd_write_char(':');
        lcd_write_char('0' + (time.seconds / 10));
        lcd_write_char('0' + (time.seconds % 10));

        /* Line 2: Date with weekday */
        lcd_set_cursor(1, 0);
        /* Show weekday (0-6, adjust index if your RTC uses 1-7) */
        if (date.weekday <= 6) {
            lcd_write_string(weekdays[date.weekday]);
        } else {
            lcd_write_string("---");
        }
        lcd_write_char(' ');

        /* Show date */
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
