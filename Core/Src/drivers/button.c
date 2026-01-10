/**
  ******************************************************************************
  * @file    button.c
  * @brief   Push button driver implementation.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "button.h"
#include "board_config.h"
#include "systick.h"
#include "stm32f4xx.h"

extern void sleep_manager_wake(void);
extern bool sleep_manager_is_sleeping(void);

/* Private typedef -----------------------------------------------------------*/

/**
  * @brief  Button state machine states.
  */
typedef enum {
    BTN_STATE_IDLE = 0,
    BTN_STATE_DEBOUNCING,
    BTN_STATE_PRESSED,
    BTN_STATE_RELEASED,
    BTN_STATE_LONG_PRESS
} button_state_t;

/**
  * @brief  Button control structure.
  */
typedef struct {
    button_state_t state;           /*!< Current state */
    bool last_raw_state;            /*!< Last GPIO reading */
    uint32_t state_enter_time;      /*!< When we entered current state */
    uint32_t press_start_time;      /*!< When button was first pressed */
    button_event_t pending_event;   /*!< Event to return */
    bool click_pending;             /*!< First click detected */
} button_ctrl_t;

/* Private variables ---------------------------------------------------------*/
static button_ctrl_t btn = {0};

/* Private function prototypes -----------------------------------------------*/
static void process_idle_state(void);
static void process_debouncing_state(void);
static void process_pressed_state(void);
static void process_released_state(void);
static void handle_press_detected(void);
static void handle_release_detected(void);

/* Exported functions --------------------------------------------------------*/

void button_init(void) {
    /* 1. Enable GPIOA clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* 2. Configure PA0 as input with pull-up */
    GPIOA->MODER &= ~GPIO_MODER_MODER0;     /* Input mode */
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR0;     /* Clear pull settings */


    /* 3. Enable SYSCFG clock for EXTI */
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    /* 4. Connect PA0 to EXTI0 */
    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PA;

    /* 5. Configure EXTI line 0 */
    EXTI->IMR |= EXTI_IMR_MR0;      /* Unmask EXTI0 */
    EXTI->FTSR |= EXTI_FTSR_TR0;    /* Falling edge trigger (release) */
    EXTI->RTSR |= EXTI_RTSR_TR0;    /* Rising edge trigger (press) */

    /* 6. Enable EXTI0 interrupt in NVIC */
    NVIC_SetPriority(EXTI0_IRQn, 0);
    NVIC_EnableIRQ(EXTI0_IRQn);

    /* 7. Initialize control structure */
    btn.state = BTN_STATE_IDLE;
    btn.last_raw_state = button_is_pressed_raw();
    btn.state_enter_time = systick_get_ticks();
    btn.pending_event = BUTTON_EVENT_NONE;
    btn.click_pending = false;
}

bool button_is_pressed_raw(void) {
	/* Active-high button: 1 = pressed, 0 = released */
    return (GPIOA->IDR & GPIO_IDR_ID0) != 0;
}

bool button_is_pressed(void) {
    return (btn.state == BTN_STATE_PRESSED || btn.state == BTN_STATE_LONG_PRESS);
}

button_event_t button_get_event(void) {
    button_event_t event = btn.pending_event;
    btn.pending_event = BUTTON_EVENT_NONE;
    return event;
}

void button_update(void) {
    uint32_t current_time = systick_get_ticks();

    switch (btn.state) {
        case BTN_STATE_IDLE:
            process_idle_state();
            break;

        case BTN_STATE_DEBOUNCING:
            process_debouncing_state();
            break;

        case BTN_STATE_PRESSED:
            process_pressed_state();
            break;

        case BTN_STATE_RELEASED:
            process_released_state();
            break;

        case BTN_STATE_LONG_PRESS:
            /* Stay in long press until button is released */
            if (!button_is_pressed_raw()) {
                btn.state = BTN_STATE_DEBOUNCING;
                btn.state_enter_time = current_time;
            }
            break;
    }

    /* Check for double-click timeout */
    if (btn.click_pending) {
        if (current_time - btn.state_enter_time >= DOUBLE_CLICK_MAX_MS) {
            /* Timeout - it was just a single click */
            btn.click_pending = false;
        }
    }
}

void button_exti_handler(void) {
    /* Check if EXTI0 triggered */
    if (EXTI->PR & EXTI_PR_PR0) {
        /* Clear pending bit */
        EXTI->PR = EXTI_PR_PR0;

        /* Update button state based on interrupt */
        bool pressed = button_is_pressed_raw();

        if (pressed && btn.state == BTN_STATE_IDLE) {
            /* Press detected - start de-bouncing */
            btn.state = BTN_STATE_DEBOUNCING;
            btn.state_enter_time = systick_get_ticks();
            btn.press_start_time = btn.state_enter_time;
        }
        else if (!pressed && (btn.state == BTN_STATE_PRESSED ||
                              btn.state == BTN_STATE_LONG_PRESS)) {
            /* Release detected - start de-bouncing */
            btn.state = BTN_STATE_DEBOUNCING;
            btn.state_enter_time = systick_get_ticks();
        }
    }
}

/* Private functions ---------------------------------------------------------*/

static void process_idle_state(void) {
    /* Nothing to do in idle state */
}

static void process_debouncing_state(void) {
    uint32_t current_time = systick_get_ticks();

    /* Wait for de-bounce time to pass */
    if (current_time - btn.state_enter_time >= DEBOUNCE_TIME_MS) {
        bool pressed = button_is_pressed_raw();

        if (pressed) {
            /* Press is stable */
            btn.state = BTN_STATE_PRESSED;
            handle_press_detected();
        } else {
            /* Release is stable */
            btn.state = BTN_STATE_RELEASED;
            handle_release_detected();
        }

        btn.state_enter_time = current_time;
    }
}

static void process_pressed_state(void) {
    uint32_t current_time = systick_get_ticks();
    uint32_t press_duration = current_time - btn.press_start_time;

    /* Check for long press */
    if (press_duration >= LONG_PRESS_TIME_MS) {
        btn.state = BTN_STATE_LONG_PRESS;
        btn.pending_event = BUTTON_EVENT_LONG_PRESS;
        btn.state_enter_time = current_time;
    }

    /* Check if button was released (via polling) */
    if (!button_is_pressed_raw()) {
        btn.state = BTN_STATE_DEBOUNCING;
        btn.state_enter_time = current_time;
    }
}

static void process_released_state(void) {
    /* Release processing complete */
    btn.state = BTN_STATE_IDLE;
}

static void handle_press_detected(void) {
    btn.pending_event = BUTTON_EVENT_PRESSED;
}

static void handle_release_detected(void) {
    uint32_t press_duration = systick_get_ticks() - btn.press_start_time;

    if (press_duration < LONG_PRESS_TIME_MS) {
        /* It was a short press */
        if (btn.click_pending) {
            /* Second click within time window = double click */
            btn.pending_event = BUTTON_EVENT_DOUBLE_CLICK;
            btn.click_pending = false;
        } else {
            /* First click - could be start of double click */
            btn.pending_event = BUTTON_EVENT_RELEASED;
            btn.click_pending = true;
            btn.state_enter_time = systick_get_ticks();
        }
    } else {
        /* It was a long press (already handled) */
        btn.pending_event = BUTTON_EVENT_RELEASED;
        btn.click_pending = false;
    }
}

/******************************** END OF FILE *********************************/
