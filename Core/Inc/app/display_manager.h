#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <stdbool.h>

/* Display Layouts */
typedef enum {
    LAYOUT_TIME_ONLY = 0,      /* Line 1: Time only */
    LAYOUT_DATE_ONLY,          /* Line 1: Date only */
    LAYOUT_TIME_DATE,          /* Line 1: Time, Line 2: Date */
    LAYOUT_TIME_WEEKDAY,       /* Line 1: Time, Line 2: Weekday */
    LAYOUT_FULL,               /* Line 1: Time + Icon, Line 2: Date + Weekday */
    LAYOUT_ALARM_FOCUS,        /* Line 1: Time + Bell, Line 2: Alarm time */
    LAYOUT_COUNT               /* Total number of layouts */
} display_layout_t;

/* Public Functions */
void display_init_custom_chars(void);
void display_set_layout(display_layout_t layout);
void display_update_time(const char* time_str);
void display_update_date(const char* date_str);
void display_show_alarm_icon(bool show);
void display_set_alarm_status(bool enabled, bool triggered);

/* Optional Helper Functions */
display_layout_t display_get_current_layout(void);
void display_next_layout(void);

#endif /* DISPLAY_MANAGER_H */
