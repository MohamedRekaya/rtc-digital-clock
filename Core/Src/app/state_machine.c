#include "state_machine.h"
#include <stddef.h>

void state_machine_init(state_machine_t* sm) {
    if (sm == NULL) return;

    sm->current_state = STATE_STANDARD;
    sm->menu_index = 0;
    sm->edit_value = 0;
    sm->display_update_needed = true;
}

void state_machine_process_button(state_machine_t* sm, button_event_t btn_event) {
    if (sm == NULL) return;

    sm->display_update_needed = true;

    switch (btn_event) {
        case BUTTON_EVENT_SHORT_PRESS:
            switch (sm->current_state) {
                case STATE_STANDARD:
                    sm->current_state = STATE_MENU;
                    sm->menu_index = 0;
                    break;

                case STATE_MENU:
                    sm->menu_index = (sm->menu_index + 1) % 4;
                    break;

                case STATE_EDIT:
                    sm->edit_value = (sm->edit_value + 1) % 100;
                    break;

                default:
                    sm->current_state = STATE_STANDARD;
                    break;
            }
            break;

        case BUTTON_EVENT_LONG_PRESS:
            switch (sm->current_state) {
                case STATE_MENU:
                    sm->current_state = STATE_EDIT;
                    sm->edit_value = 0;
                    break;

                case STATE_EDIT:
                    sm->edit_value = (sm->edit_value > 0) ? sm->edit_value - 1 : 99;
                    break;

                default:
                    // In STANDARD state, long press is handled in main.c for layout cycling
                    break;
            }
            break;

        case BUTTON_EVENT_DOUBLE_CLICK:
            switch (sm->current_state) {
                case STATE_MENU:
                    sm->current_state = STATE_STANDARD;
                    break;

                case STATE_EDIT:
                    sm->current_state = STATE_MENU;
                    break;

                default:
                    // In STANDARD state, double click is handled in main.c for alarm toggle
                    break;
            }
            break;

        default:
            break;
    }
}

system_state_t state_machine_get_current_state(const state_machine_t* sm) {
    return (sm != NULL) ? sm->current_state : STATE_STANDARD;
}
