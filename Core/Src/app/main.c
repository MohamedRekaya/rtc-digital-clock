#include "systick.h"
#include "lcd1602_i2c.h"
#include "i2c.h"
#include "rtc.h"
#include "display_manager.h"
#include <string.h>
#include <stdio.h>

// Application state
typedef struct {
    uint32_t layout_change_time;
    uint8_t current_layout;
    bool alarm_enabled;
    bool alarm_triggered;
    bool display_updated;  // Flag to indicate display needs refresh
} app_state_t;

static app_state_t app_state = {
    .layout_change_time = 0,
    .current_layout = 0,
    .alarm_enabled = true,
    .alarm_triggered = false,
    .display_updated = false
};

// ============================================
// TIME/DATE FORMATTING FUNCTIONS
// ============================================

static void format_time_from_rtc(char* buffer, size_t buffer_size) {
    rtc_time_t time;
    rtc_get_time(&time);
    snprintf(buffer, buffer_size, "%02u:%02u:%02u",
            (unsigned)time.hours,
            (unsigned)time.minutes,
            (unsigned)time.seconds);
}

static void format_date_from_rtc(char* buffer, size_t buffer_size) {
    rtc_date_t date;
    rtc_get_date(&date);
    snprintf(buffer, buffer_size, "%02u/%02u/%04u",
            (unsigned)date.day,
            (unsigned)date.month,
            (unsigned)date.year);
}

// ============================================
// UPDATE DISPLAY FROM RTC
// ============================================

static void update_display_from_rtc(void) {
    char time_str[12];
    char date_str[15];

    // Get formatted data from RTC
    format_time_from_rtc(time_str, sizeof(time_str));
    format_date_from_rtc(date_str, sizeof(date_str));

    // Update display model
    display_update_time(time_str);
    display_update_date(date_str);

    // Mark display for refresh
    app_state.display_updated = true;
}

// ============================================
// AUTOMATIC LAYOUT CYCLING
// ============================================

static void cycle_layouts(void) {
    uint32_t current_time = systick_get_ticks();

    // Change layout every 5 seconds
    if (current_time - app_state.layout_change_time >= 5000) {
        switch (app_state.current_layout) {
            case 0:
                display_set_layout(LAYOUT_TIME_ONLY);
                display_show_alarm_icon(false);
                break;
            case 1:
                display_set_layout(LAYOUT_DATE_ONLY);
                display_show_alarm_icon(false);
                break;
            case 2:
                display_set_layout(LAYOUT_TIME_DATE);
                display_show_alarm_icon(false);
                break;
            case 3:
                display_set_layout(LAYOUT_TIME_WEEKDAY);
                display_show_alarm_icon(false);
                break;
            case 4:
                display_set_layout(LAYOUT_FULL);
                display_show_alarm_icon(true);
                display_set_alarm_status(app_state.alarm_enabled, app_state.alarm_triggered);
                break;
            case 5:
                display_set_layout(LAYOUT_ALARM_FOCUS);
                display_show_alarm_icon(true);
                display_set_alarm_status(app_state.alarm_enabled, app_state.alarm_triggered);
                break;
        }

        app_state.current_layout = (app_state.current_layout + 1) % 6;
        app_state.layout_change_time = current_time;

        // Layout change needs display refresh
        app_state.display_updated = true;
    }
}

// ============================================
// RTC PERIODIC CALLBACK (Called from interrupt)
// ============================================

void rtc_periodic_callback(void) {
    update_display_from_rtc();  // Updates data and sets flag
}

// ============================================
// MAIN APPLICATION WITH PERIODIC INTERRUPT
// ============================================

int main(void) {
    // Initialize hardware
    systick_init();
    i2c_init();
    lcd_init();
    lcd_backlight_on();

    // Initialize display manager
    display_init();

    // Initialize RTC
    rtc_init();

    // Startup message
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_write_string("RTC Clock");
    lcd_set_cursor(1, 0);
    lcd_write_string("Starting...");
    systick_delay_ms(2000);
    lcd_clear();

    // Setup RTC periodic interrupt (every second)
    rtc_periodic_init(RTC_PERIODIC_EVERY_SECOND);
    rtc_periodic_enable();



    // Initial display setup
    display_set_layout(LAYOUT_TIME_DATE);
    display_show_alarm_icon(true);
    display_set_alarm_status(app_state.alarm_enabled, app_state.alarm_triggered);

    // Get initial time
    update_display_from_rtc();

    // Initial refresh
    display_refresh();
    app_state.display_updated = false;

    // Main loop
    while (1) {


        // 1. Auto-cycle layouts every 5 seconds
        cycle_layouts();



        // 3. Refresh display if updated
        if (app_state.display_updated) {
            display_refresh();  // Simple refresh - no partial/full complexity
            app_state.display_updated = false;
        }

        // 4. Small delay
        systick_delay_ms(10);
    }
}
