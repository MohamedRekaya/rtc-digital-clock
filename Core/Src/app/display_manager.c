#include "display_manager.h"
#include "lcd1602_i2c.h"
#include "custom_chars.h"
#include <string.h>
#include <stdio.h>

// ============================================
// DISPLAY STATE
// ============================================

static display_state_t display_state = {
    .current_layout = LAYOUT_TIME_DATE,
    .alarm_enabled = false,
    .alarm_triggered = false,
    .alarm_icon_visible = false,
    .time_buffer = "00:00:00",
    .date_buffer = "01/01/2000",
    .weekday_buffer = "Monday",
    .alarm_time_buffer = "00:00"
};

// ============================================
// PRIVATE HELPER FUNCTIONS
// ============================================

// Draw combined date+weekday for FULL layout
static void draw_full_layout_line2(void) {
    char buffer[17];
    char short_weekday[4];

    // Get first 3 chars of weekday
    strncpy(short_weekday, display_state.weekday_buffer, 3);
    short_weekday[3] = '\0';

    snprintf(buffer, sizeof(buffer), "%s %s",
             display_state.date_buffer, short_weekday);
    lcd_set_cursor(1, 0);
    lcd_write_string(buffer);
}

// ============================================
// PUBLIC FUNCTIONS
// ============================================

void display_init(void) {
    // Load custom characters into LCD CGRAM
    lcd_create_char(LCD_CUSTOM_BELL, bell_char);
    lcd_create_char(LCD_CUSTOM_ALARM_ON, alarm_on_char);
    lcd_create_char(LCD_CUSTOM_ALARM_OFF, alarm_off_char);
    lcd_create_char(LCD_CUSTOM_CHECK, check_char);
    lcd_create_char(LCD_CUSTOM_CROSS, cross_char);
    lcd_create_char(LCD_CUSTOM_CLOCK, clock_char);
    lcd_create_char(LCD_CUSTOM_CALENDAR, calendar_char);
    lcd_create_char(LCD_CUSTOM_SETTINGS, settings_char);

    // Initialize display state buffers with safe values
    strcpy(display_state.time_buffer, "00:00:00");
    strcpy(display_state.date_buffer, "01/01/2000");
    strcpy(display_state.weekday_buffer, "Monday");
    strcpy(display_state.alarm_time_buffer, "00:00");
}

void display_set_layout(display_layout_t layout) {
    if (layout != display_state.current_layout && layout < LAYOUT_COUNT) {
        display_state.current_layout = layout;
    }
}

void display_update_time(const char* time_str) {
    if (time_str != NULL && strlen(time_str) <= 8) {
        strcpy(display_state.time_buffer, time_str);
    }
}

void display_update_date(const char* date_str) {
    if (date_str != NULL && strlen(date_str) <= 10) {
        strcpy(display_state.date_buffer, date_str);
    }
}

void display_update_weekday(const char* weekday_str) {
    if (weekday_str != NULL && strlen(weekday_str) < sizeof(display_state.weekday_buffer)) {
        strcpy(display_state.weekday_buffer, weekday_str);
    }
}

void display_show_alarm_icon(bool show) {
    display_state.alarm_icon_visible = show;
}

void display_set_alarm_status(bool enabled, bool triggered) {
    display_state.alarm_enabled = enabled;
    display_state.alarm_triggered = triggered;
}

void display_set_alarm_time(const char* alarm_time_str) {
    if (alarm_time_str != NULL && strlen(alarm_time_str) <= 5) {
        strcpy(display_state.alarm_time_buffer, alarm_time_str);
    }
}

// ============================================
// DISPLAY REFRESH FUNCTION (SIMPLE!)
// ============================================

void display_refresh(void) {
    lcd_clear();

    switch (display_state.current_layout) {
        case LAYOUT_TIME_ONLY:
            // Time only (centered)
            lcd_set_cursor(0, (16 - strlen(display_state.time_buffer)) / 2);
            lcd_write_string(display_state.time_buffer);
            break;

        case LAYOUT_DATE_ONLY:
            // Date only (centered)
            lcd_set_cursor(0, (16 - strlen(display_state.date_buffer)) / 2);
            lcd_write_string(display_state.date_buffer);
            break;

        case LAYOUT_TIME_DATE:
            // Line 1: Time
            lcd_set_cursor(0, 0);
            lcd_write_string(display_state.time_buffer);

            // Line 2: Date
            lcd_set_cursor(1, 0);
            lcd_write_string(display_state.date_buffer);
            break;

        case LAYOUT_TIME_WEEKDAY:
            // Line 1: Time
            lcd_set_cursor(0, 0);
            lcd_write_string(display_state.time_buffer);

            // Line 2: Weekday (centered)
            lcd_set_cursor(1, (16 - strlen(display_state.weekday_buffer)) / 2);
            lcd_write_string(display_state.weekday_buffer);
            break;

        case LAYOUT_FULL:
            // Line 1: Time + Alarm icon
            lcd_set_cursor(0, 0);
            lcd_write_string(display_state.time_buffer);

            if (display_state.alarm_icon_visible) {
                lcd_set_cursor(0, 15);
                if (display_state.alarm_enabled) {
                    lcd_write_custom_char(LCD_CUSTOM_ALARM_ON);
                } else {
                    lcd_write_custom_char(LCD_CUSTOM_ALARM_OFF);
                }
            }

            // Line 2: Date + Weekday (abbreviated)
            draw_full_layout_line2();
            break;

        case LAYOUT_ALARM_FOCUS:
            // Line 1: Time + Bell icon
            lcd_set_cursor(0, 0);
            lcd_write_string(display_state.time_buffer);

            if (display_state.alarm_icon_visible) {
                lcd_set_cursor(0, 15);
                if (display_state.alarm_triggered) {
                    lcd_write_custom_char(LCD_CUSTOM_ALARM_ON);
                } else if (display_state.alarm_enabled) {
                    lcd_write_custom_char(LCD_CUSTOM_BELL);
                } else {
                    lcd_write_custom_char(LCD_CUSTOM_ALARM_OFF);
                }
            }

            // Line 2: Alarm time
            lcd_set_cursor(1, 0);
            lcd_write_string("Alarm: ");
            lcd_write_string(display_state.alarm_time_buffer);
            break;

        default:
            // Invalid layout - show error
            lcd_set_cursor(0, 0);
            lcd_write_string("Invalid Layout");
            display_state.current_layout = LAYOUT_TIME_DATE;
            break;
    }
}

// ============================================
// UTILITY FUNCTIONS
// ============================================

display_layout_t display_get_current_layout(void) {
    return display_state.current_layout;
}

void display_next_layout(void) {
    display_state.current_layout = (display_state.current_layout + 1) % LAYOUT_COUNT;
}

const display_state_t* display_get_state(void) {
    return &display_state;
}
