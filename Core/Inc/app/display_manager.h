#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <stdbool.h>

// Layout definitions
typedef enum {
    LAYOUT_TIME_ONLY,
    LAYOUT_DATE_ONLY,
    LAYOUT_TIME_DATE,
    LAYOUT_TIME_WEEKDAY,
    LAYOUT_FULL,
    LAYOUT_ALARM_FOCUS,
    LAYOUT_COUNT
} display_layout_t;

// Display state structure
typedef struct {
    char time_buffer[9];        // HH:MM:SS
    char date_buffer[11];       // DD/MM/YYYY
    char weekday_buffer[10];    // Monday
    char alarm_time_buffer[6];  // HH:MM
    display_layout_t current_layout;
    bool alarm_enabled;
    bool alarm_triggered;
    bool alarm_icon_visible;
} display_state_t;



// ===== PUBLIC API =====

// Initialization
void display_init(void);

// Model update functions
void display_set_layout(display_layout_t layout);
void display_update_time(const char* time_str);
void display_update_date(const char* date_str);
void display_update_weekday(const char* weekday_str);
void display_show_alarm_icon(bool show);
void display_set_alarm_status(bool enabled, bool triggered);
void display_set_alarm_time(const char* alarm_time_str);

// View refresh function
void display_refresh(void);

// Utility functions
display_layout_t display_get_current_layout(void);
void display_next_layout(void);
const display_state_t* display_get_state(void);

#endif // DISPLAY_MANAGER_H
