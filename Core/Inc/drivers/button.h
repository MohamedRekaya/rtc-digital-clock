/**
  ******************************************************************************
  * @file    button.h
  * @brief   Push button driver with EXTI interrupt.
  ******************************************************************************
  */
#ifndef BUTTON_H
#define BUTTON_H


/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/

/**
  * @brief  Button events for application.
  */
typedef enum {
    BUTTON_EVENT_NONE = 0,    /*!< No event detected */
    BUTTON_EVENT_PRESSED,     /*!< Button pressed (short) */
    BUTTON_EVENT_RELEASED,    /*!< Button released */
    BUTTON_EVENT_LONG_PRESS,  /*!< Long press (2 seconds) */
    BUTTON_EVENT_DOUBLE_CLICK /*!< Double click detected */
} button_event_t;

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Initialize button GPIO and EXTI interrupt.
  * @note   Must be called after systick_init().
  * @retval None
  */
void button_init(void);

/**
  * @brief  Get current button state (raw GPIO reading).
  * @retval true if button is pressed, false if released.
  */
bool button_is_pressed_raw(void);

/**
  * @brief  Get de-bounced button state.
  * @retval true if button is pressed, false if released.
  */
bool button_is_pressed(void);

/**
  * @brief  Get button event (non-blocking).
  * @note   Call periodically in main loop.
  * @retval button_event_t Detected event.
  */
button_event_t button_get_event(void);

/**
  * @brief  Update button state machine (non-blocking).
  * @note   Call periodically (e.g., every 10ms) in main loop.
  * @retval None
  */
void button_update(void);

/**
  * @brief  EXTI interrupt handler for button.
  * @note   Call from EXTI0_IRQHandler().
  * @retval None
  */
void button_exti_handler(void);



#endif /* BUTTON_H */

/******************************** END OF FILE *********************************/
