#include "display_manager.h"
#include "lcd1602_i2c.h"
#include "custom_chars.h"
#include "rtc.h"
#include <string.h>
#include <stdio.h>

// Current display state
static display_layout_t current_layout = LAYOUT_TIME_DATE;
static bool alarm_enabled = false;
static bool alarm_triggered = false;
static bool alarm_icon_visible = false;

// Display buffers
static char time_buffer[9] = "00:00:00";    // HH:MM:SS
static char date_buffer[11] = "01/01/2000"; // DD/MM/YYYY
static char weekday_buffer[10] = "Monday";  // Weekday name
static char alarm_time_buffer[6] = "00:00"; // Alarm time HH:MM

// ============================================
// PRIVATE HELPER FUNCTIONS
// ============================================

// Get weekday name from RTC weekday number (1=Monday, 7=Sunday)
static const char* get_weekday_name(uint8_t weekday) {
    switch (weekday) {
        case 1: return "Monday";
        case 2: return "Tuesday";
        case 3: return "Wednesday";
        case 4: return "Thursday";
        case 5: return "Friday";
        case 6: return "Saturday";
        case 7: return "Sunday";
        default: return "Unknown";
    }
}

// Update weekday buffer from RTC
static void update_weekday_from_rtc(void) {
    rtc_date_t date;
    rtc_get_date(&date);
    const char* weekday_name = get_weekday_name(date.weekday);
    strncpy(weekday_buffer, weekday_name, sizeof(weekday_buffer) - 1);
    weekday_buffer[sizeof(weekday_buffer) - 1] = '\0';
}

// Refresh display based on current layout
static void refresh_display(void) {
    lcd_clear();

    switch (current_layout) {
        case LAYOUT_TIME_ONLY:
            // Line 1: Time only (centered)
            lcd_set_cursor(0, (16 - strlen(time_buffer)) / 2);
            lcd_write_string(time_buffer);
            break;

        case LAYOUT_DATE_ONLY:
            // Line 1: Date only (centered)
            lcd_set_cursor(0, (16 - strlen(date_buffer)) / 2);
            lcd_write_string(date_buffer);
            break;

        case LAYOUT_TIME_DATE:
            // Line 1: Time
            lcd_set_cursor(0, 0);
            lcd_write_string(time_buffer);

            // Line 2: Date
            lcd_set_cursor(1, 0);
            lcd_write_string(date_buffer);
            break;

        case LAYOUT_TIME_WEEKDAY:
            // Line 1: Time
            lcd_set_cursor(0, 0);
            lcd_write_string(time_buffer);

            // Line 2: Weekday (centered)
            update_weekday_from_rtc();
            lcd_set_cursor(1, (16 - strlen(weekday_buffer)) / 2);
            lcd_write_string(weekday_buffer);
            break;

        case LAYOUT_FULL:
            // Line 1: Time + Alarm icon
            lcd_set_cursor(0, 0);
            lcd_write_string(time_buffer);

            if (alarm_icon_visible) {
                lcd_set_cursor(0, 15);
                if (alarm_enabled) {
                    lcd_write_custom_char(LCD_CUSTOM_ALARM_ON);
                } else {
                    lcd_write_custom_char(LCD_CUSTOM_ALARM_OFF);
                }
            }

            // Line 2: Date + Weekday (abbreviated)
            char date_week_buffer[17];
            char short_weekday[4];
            update_weekday_from_rtc();
            strncpy(short_weekday, weekday_buffer, 3);
            short_weekday[3] = '\0';

            snprintf(date_week_buffer, sizeof(date_week_buffer), "%s %s",
                     date_buffer, short_weekday);
            lcd_set_cursor(1, 0);
            lcd_write_string(date_week_buffer);
            break;

        case LAYOUT_ALARM_FOCUS:
            // Line 1: Time + Bell icon
            lcd_set_cursor(0, 0);
            lcd_write_string(time_buffer);

            lcd_set_cursor(0, 15);
            if (alarm_triggered) {
                lcd_write_custom_char(LCD_CUSTOM_ALARM_ON); // Flashing bell when triggered
            } else if (alarm_enabled) {
                lcd_write_custom_char(LCD_CUSTOM_BELL); // Normal bell
            } else {
                lcd_write_custom_char(LCD_CUSTOM_ALARM_OFF); // Empty bell
            }

            // Line 2: Alarm time
            lcd_set_cursor(1, 0);
            lcd_write_string("Alarm: ");
            lcd_write_string(alarm_time_buffer);
            break;

        default:
            // Handle LAYOUT_COUNT or any invalid layout
            lcd_set_cursor(0, 0);
            lcd_write_string("Invalid Layout");
            break;
    }
}

// ============================================
// PUBLIC FUNCTIONS
// ============================================

void display_init_custom_chars(void) {
    // Load all custom characters into LCD CGRAM
    lcd_create_char(LCD_CUSTOM_BELL, bell_char);
    lcd_create_char(LCD_CUSTOM_ALARM_ON, alarm_on_char);
    lcd_create_char(LCD_CUSTOM_ALARM_OFF, alarm_off_char);
    lcd_create_char(LCD_CUSTOM_CHECK, check_char);
    lcd_create_char(LCD_CUSTOM_CROSS, cross_char);
    lcd_create_char(LCD_CUSTOM_CLOCK, clock_char);
    lcd_create_char(LCD_CUSTOM_CALENDAR, calendar_char);
    lcd_create_char(LCD_CUSTOM_SETTINGS, settings_char);
}

void display_set_layout(display_layout_t layout) {
    if (layout != current_layout) {
        current_layout = layout;
        refresh_display();
    }
}

void display_update_time(const char* time_str) {
    if (time_str != NULL && strlen(time_str) <= 8) {
        strncpy(time_buffer, time_str, sizeof(time_buffer) - 1);
        time_buffer[sizeof(time_buffer) - 1] = '\0';

        // Only refresh if we're in a layout that shows time
        if (current_layout == LAYOUT_TIME_ONLY ||
            current_layout == LAYOUT_TIME_DATE ||
            current_layout == LAYOUT_TIME_WEEKDAY ||
            current_layout == LAYOUT_FULL ||
            current_layout == LAYOUT_ALARM_FOCUS) {

            // For time-only or time-date layouts, update the time position
            if (current_layout == LAYOUT_TIME_ONLY) {
                lcd_set_cursor(0, (16 - strlen(time_buffer)) / 2);
                lcd_write_string(time_buffer);
            }
            else if (current_layout == LAYOUT_TIME_DATE ||
                     current_layout == LAYOUT_TIME_WEEKDAY ||
                     current_layout == LAYOUT_FULL ||
                     current_layout == LAYOUT_ALARM_FOCUS) {
                lcd_set_cursor(0, 0);
                lcd_write_string(time_buffer);
            }
        }
    }
}

void display_update_date(const char* date_str) {
    if (date_str != NULL && strlen(date_str) <= 10) {
        strncpy(date_buffer, date_str, sizeof(date_buffer) - 1);
        date_buffer[sizeof(date_buffer) - 1] = '\0';

        // Only refresh if we're in a layout that shows date
        if (current_layout == LAYOUT_DATE_ONLY ||
            current_layout == LAYOUT_TIME_DATE ||
            current_layout == LAYOUT_FULL) {

            if (current_layout == LAYOUT_DATE_ONLY) {
                lcd_set_cursor(0, (16 - strlen(date_buffer)) / 2);
                lcd_write_string(date_buffer);
            }
            else if (current_layout == LAYOUT_TIME_DATE) {
                lcd_set_cursor(1, 0);
                lcd_write_string(date_buffer);
            }
            else if (current_layout == LAYOUT_FULL) {
                // Update the combined date+weekday line
                char date_week_buffer[17];
                char short_weekday[4];
                strncpy(short_weekday, weekday_buffer, 3);
                short_weekday[3] = '\0';

                snprintf(date_week_buffer, sizeof(date_week_buffer), "%s %s",
                         date_buffer, short_weekday);
                lcd_set_cursor(1, 0);
                lcd_write_string(date_week_buffer);
            }
        }
    }
}

void display_show_alarm_icon(bool show) {
    if (alarm_icon_visible != show) {
        alarm_icon_visible = show;

        // Only update if in a layout that shows alarm icon
        if (current_layout == LAYOUT_FULL || current_layout == LAYOUT_ALARM_FOCUS) {
            lcd_set_cursor(0, 15);
            if (show) {
                if (alarm_triggered) {
                    lcd_write_custom_char(LCD_CUSTOM_ALARM_ON);
                } else if (alarm_enabled) {
                    lcd_write_custom_char(LCD_CUSTOM_BELL);
                } else {
                    lcd_write_custom_char(LCD_CUSTOM_ALARM_OFF);
                }
            } else {
                lcd_write_char(' ');
            }
        }
    }
}

void display_set_alarm_status(bool enabled, bool triggered) {
    bool needs_update = false;

    if (alarm_enabled != enabled) {
        alarm_enabled = enabled;
        needs_update = true;
    }

    if (alarm_triggered != triggered) {
        alarm_triggered = triggered;
        needs_update = true;
    }

    // Update alarm time (placeholder - you would get this from RTC)
    if (enabled) {
        // For now, use a default alarm time
        // In a real application, you would get this from RTC alarm registers
        snprintf(alarm_time_buffer, sizeof(alarm_time_buffer), "07:30");
    }

    if (needs_update && alarm_icon_visible) {
        // Update the alarm icon if visible
        lcd_set_cursor(0, 15);
        if (triggered) {
            lcd_write_custom_char(LCD_CUSTOM_ALARM_ON);
        } else if (enabled) {
            lcd_write_custom_char(LCD_CUSTOM_BELL);
        } else {
            lcd_write_custom_char(LCD_CUSTOM_ALARM_OFF);
        }
    }
}

// Optional helper function to get current layout
display_layout_t display_get_current_layout(void) {
    return current_layout;
}

// Optional helper function to cycle through layouts
void display_next_layout(void) {
    current_layout = (current_layout + 1) % LAYOUT_COUNT;
    refresh_display();
}
