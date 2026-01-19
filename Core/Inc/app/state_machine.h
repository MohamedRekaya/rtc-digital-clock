#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "button.h"

typedef enum {
    STATE_STANDARD = 0,    // Show time/date
    STATE_MENU,            // Menu navigation
    STATE_EDIT,            // Edit value
    STATE_COUNT
} system_state_t;

typedef struct {
    system_state_t current_state;
    uint8_t menu_index;
    uint8_t edit_value;
    bool display_update_needed;
} state_machine_t;

void state_machine_init(state_machine_t* sm);
void state_machine_process_button(state_machine_t* sm, button_event_t btn_event);
system_state_t state_machine_get_current_state(const state_machine_t* sm);

#endif
