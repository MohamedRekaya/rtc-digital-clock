#include "button.h"
#include "systick.h"
#include "lcd1602_i2c.h"
#include "i2c.h"
#include "rtc.h"
#include "state_machine.h"
#include "display_manager.h"
#include <stdio.h>
#include <string.h>

static state_machine_t sm;
static uint32_t last_blink_time = 0;
static bool blink_state = true;

// Time/date buffers for display manager
static char time_str[12] = "00:00:00";
static char date_str[15] = "01/01/2000";

// Alarm state
static bool alarm_enabled = true;
static bool alarm_triggered = false;

// ============================================
// DISPLAY UPDATE FUNCTION
// ============================================

static void update_display(void) {
    switch (sm.current_state) {
        case STATE_STANDARD: {
            // Get current time from RTC
            rtc_time_t time;
            rtc_date_t date;
            rtc_get_time(&time);
            rtc_get_date(&date);

            // Format time for display manager
            snprintf(time_str, sizeof(time_str), "%02u:%02u:%02u",
                    (unsigned)time.hours,
                    (unsigned)time.minutes,
                    (unsigned)time.seconds);

            // Format date for display manager
            snprintf(date_str, sizeof(date_str), "%02u/%02u/%04u",
                    (unsigned)date.day,
                    (unsigned)date.month,
                    (unsigned)date.year);

            // Update display manager
            display_update_time(time_str);
            display_update_date(date_str);

            // Show alarm icon and status (using ALARM_FOCUS layout)
            display_show_alarm_icon(true);
            display_set_alarm_status(alarm_enabled, alarm_triggered);

            break;
        }

        case STATE_MENU: {
            // For menu state, direct LCD update
            char line1[17] = {0};
            char line2[17] = {0};
            const char* menu_items[] = {"Set Time", "Set Date", "Set Alarm", "Alarm On/Off"};

            strcpy(line1, "Menu");
            if (sm.menu_index < 4) {
                snprintf(line2, sizeof(line2), ">%s", menu_items[sm.menu_index]);
            }

            // Direct LCD update for menu
            lcd_set_cursor(0, 0);
            lcd_write_string(line1);
            lcd_set_cursor(1, 0);
            lcd_write_string(line2);

            // Clear any leftover characters
            if (strlen(line1) < 16) {
                lcd_set_cursor(0, strlen(line1));
                for (uint8_t i = strlen(line1); i < 16; i++) {
                    lcd_write_char(' ');
                }
            }
            if (strlen(line2) < 16) {
                lcd_set_cursor(1, strlen(line2));
                for (uint8_t i = strlen(line2); i < 16; i++) {
                    lcd_write_char(' ');
                }
            }
            break;
        }

        case STATE_EDIT: {
            // For edit state, direct LCD update
            char line1[17] = {0};
            char line2[17] = {0};
            const char* edit_names[] = {"Time", "Date", "Alarm Time", "Alarm State"};

            if (sm.menu_index < 4) {
                snprintf(line1, sizeof(line1), "Edit %s", edit_names[sm.menu_index]);
            } else {
                strcpy(line1, "Edit");
            }

            // Special handling for alarm state editing
            if (sm.menu_index == 3) { // Alarm state edit
                if (blink_state) {
                    snprintf(line2, sizeof(line2), "[%s]", alarm_enabled ? "ON " : "OFF");
                } else {
                    snprintf(line2, sizeof(line2), " %s ", alarm_enabled ? "ON " : "OFF");
                }
            } else {
                // Normal numeric editing
                if (blink_state) {
                    snprintf(line2, sizeof(line2), "[%02d]", sm.edit_value);
                } else {
                    snprintf(line2, sizeof(line2), " %02d ", sm.edit_value);
                }
            }

            // Direct LCD update for edit
            lcd_set_cursor(0, 0);
            lcd_write_string(line1);
            lcd_set_cursor(1, 0);
            lcd_write_string(line2);

            // Clear any leftover characters
            if (strlen(line1) < 16) {
                lcd_set_cursor(0, strlen(line1));
                for (uint8_t i = strlen(line1); i < 16; i++) {
                    lcd_write_char(' ');
                }
            }
            if (strlen(line2) < 16) {
                lcd_set_cursor(1, strlen(line2));
                for (uint8_t i = strlen(line2); i < 16; i++) {
                    lcd_write_char(' ');
                }
            }
            break;
        }

        default:
            // Handle invalid state
            lcd_set_cursor(0, 0);
            lcd_write_string("Error: Bad State");
            lcd_set_cursor(1, 0);
            lcd_write_string("Press button");
            break;
    }
}

// ============================================
// RTC PERIODIC CALLBACK
// ============================================

void rtc_periodic_callback(void) {
    if (sm.current_state == STATE_STANDARD) {
        sm.display_update_needed = true;
    }
}

// ============================================
// ENHANCED STATE MACHINE HANDLING
// ============================================

// Handle alarm-specific actions
static void handle_alarm_action(void) {
    if (sm.current_state == STATE_STANDARD) {
        // Toggle alarm on double click
        alarm_enabled = !alarm_enabled;
        sm.display_update_needed = true;
    }
    else if (sm.current_state == STATE_MENU && sm.menu_index == 3) {
        // Toggle alarm from menu
        alarm_enabled = !alarm_enabled;
        sm.display_update_needed = true;
    }
    else if (sm.current_state == STATE_EDIT && sm.menu_index == 3) {
        // Toggle alarm during editing
        alarm_enabled = !alarm_enabled;
        sm.display_update_needed = true;
    }
}

// Cycle through display layouts
static void cycle_display_layout(void) {
    static display_layout_t current_layout = LAYOUT_ALARM_FOCUS;

    // Cycle through layouts
    display_layout_t layouts[] = {
        LAYOUT_ALARM_FOCUS,  // Primary alarm layout
        LAYOUT_FULL,         // Time + date + weekday + icon
        LAYOUT_TIME_DATE,    // Simple time + date
        LAYOUT_TIME_ONLY,    // Just time
        LAYOUT_DATE_ONLY,    // Just date
        LAYOUT_TIME_WEEKDAY  // Time + weekday
    };

    uint8_t layout_count = sizeof(layouts) / sizeof(layouts[0]);

    // Find current layout index
    uint8_t current_index = 0;
    for (uint8_t i = 0; i < layout_count; i++) {
        if (layouts[i] == current_layout) {
            current_index = i;
            break;
        }
    }

    // Move to next layout
    current_index = (current_index + 1) % layout_count;
    current_layout = layouts[current_index];

    // Apply layout
    display_set_layout(current_layout);

    // Force display update
    sm.display_update_needed = true;
}

// ============================================
// UPDATED BUTTON HANDLING IN MAIN
// ============================================

// In main.c, update the button event handling:

static void process_button_action(button_event_t btn_event) {
    if (btn_event == BUTTON_EVENT_NONE) return;

    state_machine_process_button(&sm, btn_event);

    // Additional custom actions
    if (sm.current_state == STATE_STANDARD) {
        if (btn_event == BUTTON_EVENT_DOUBLE_CLICK) {
            handle_alarm_action(); // Toggle alarm on double click
        }
        else if (btn_event == BUTTON_EVENT_LONG_PRESS) {
            cycle_display_layout(); // Change layout on long press
        }
    }
    else if (sm.current_state == STATE_EDIT && sm.menu_index == 3) {
        // Special handling for alarm state editing
        if (btn_event == BUTTON_EVENT_SHORT_PRESS) {
            handle_alarm_action(); // Toggle alarm state
        }
    }
}

// ============================================
// MAIN APPLICATION
// ============================================

int main(void) {
    // Initialize hardware
    systick_init();
    i2c_init();
    lcd_init();
    lcd_backlight_on();

    // Initialize display manager with custom characters
    display_init_custom_chars();

    // Set ALARM_FOCUS as default layout
    display_set_layout(LAYOUT_ALARM_FOCUS);

    // Initialize RTC
    rtc_init();

    // Setup RTC periodic updates (if enabled)
    #if RTC_PERIODIC_IRQ_ENABLE
    rtc_periodic_init(RTC_PERIODIC_EVERY_SECOND);
    rtc_periodic_enable();
    #endif

    // Initialize button
    button_init();

    // Initialize state machine
    state_machine_init(&sm);

    // Startup message
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_write_string("Alarm Clock");
    lcd_set_cursor(1, 0);
    lcd_write_string("v1.0 Ready");
    systick_delay_ms(1000);

    // Clear and set alarm layout
    lcd_clear();
    display_set_layout(LAYOUT_ALARM_FOCUS);

    // Force initial update
    sm.display_update_needed = true;

    // Main loop
    while (1) {
        uint32_t current_time = systick_get_ticks();

        // 1. Handle button
        button_update();
        button_event_t btn_event = button_get_event();

        if (btn_event != BUTTON_EVENT_NONE) {
            process_button_action(btn_event);
        }

        // 2. Handle blinking in EDIT state
        if (sm.current_state == STATE_EDIT) {
            if (current_time - last_blink_time >= 500) { // 500ms blink
                blink_state = !blink_state;
                sm.display_update_needed = true;
                last_blink_time = current_time;
            }
        }

        // 3. Check RTC periodic interrupt flag
        #if RTC_PERIODIC_IRQ_ENABLE
        if (rtc_is_periodic_triggered()) {
            rtc_clear_periodic_flag();
            rtc_periodic_callback(); // This sets display_update_needed
        }
        #endif

        // 4. Fallback time update (if no RTC interrupt)
        #if !RTC_PERIODIC_IRQ_ENABLE
        static uint32_t last_second = 0;
        if (current_time - last_second >= 1000) {
            if (sm.current_state == STATE_STANDARD) {
                sm.display_update_needed = true;
            }
            last_second = current_time;
        }
        #endif

        // 5. Check alarm (simplified - in real app, check RTC alarm)
        if (alarm_enabled && !alarm_triggered) {
            rtc_time_t current_rtc_time;
            rtc_get_time(&current_rtc_time);

            // Example: Trigger alarm at 7:30 (you'd get this from RTC alarm registers)
            if (current_rtc_time.hours == 7 && current_rtc_time.minutes == 30 && current_rtc_time.seconds == 0) {
                alarm_triggered = true;
                sm.display_update_needed = true;
            }
        }

        // 6. Snooze/dismiss alarm (example: double click when alarm is triggered)
        if (alarm_triggered && btn_event == BUTTON_EVENT_DOUBLE_CLICK) {
            alarm_triggered = false;
            sm.display_update_needed = true;
        }

        // 7. Update display if needed
        if (sm.display_update_needed) {
            update_display();
            sm.display_update_needed = false;
        }

        // 8. Small delay
        systick_delay_ms(10);
    }
}
