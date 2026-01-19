#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>
#include <stdint.h>

/* Button Events - ONLY 3 EVENTS! */
typedef enum {
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_SHORT_PRESS,   // Released after < 1s (generated after timeout)
    BUTTON_EVENT_LONG_PRESS,    // Released after ≥ 1s
    BUTTON_EVENT_DOUBLE_CLICK   // Two presses within 300ms
} button_event_t;



/* Public Functions */
void button_init(void);
bool button_is_pressed_raw(void);
bool button_is_pressed(void);
button_event_t button_get_event(void);
void button_update(void);
void button_exti_handler(void);

#endif /* BUTTON_H */
