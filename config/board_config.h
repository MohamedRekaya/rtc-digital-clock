/**
  ******************************************************************************
  * @file    board_config.h
  * @brief   Board-specific configuration for STM32F407 Discovery.
  ******************************************************************************
  */
#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

  /* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "gpio_pins.h"



/* Exported types ------------------------------------------------------------*/


/**
  * @brief  LED identifiers for Discovery board.
  */
typedef enum {
    LED_GREEN = 0,  // PD12
    LED_ORANGE,     // PD13
    LED_RED,        // PD14
    LED_BLUE,       // PD15
    LED_COUNT,
    LED_ALL = 0xFF  // Special value for all LEDs
} led_id_t;

/* Exported constants --------------------------------------------------------*/


// LED GPIO Configuration
#define LED_GPIO_PORT        GPIOD

#define LED_GPIO_CLK_ENABLE()  do {        \
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;   \
} while(0)

// LED Pin Definitions
// ===============================
// STM32F4 Discovery LEDs
// ===============================
#define LED_GREEN_PIN_NUM   GPIO_PIN_12_NUM
#define LED_GREEN_PIN_MSK   GPIO_PIN_12

#define LED_ORANGE_PIN_NUM  GPIO_PIN_13_NUM
#define LED_ORANGE_PIN_MSK  GPIO_PIN_13

#define LED_RED_PIN_NUM     GPIO_PIN_14_NUM
#define LED_RED_PIN_MSK     GPIO_PIN_14

#define LED_BLUE_PIN_NUM    GPIO_PIN_15_NUM
#define LED_BLUE_PIN_MSK    GPIO_PIN_15


#define LED_ALL_PINS     (LED_GREEN_PIN_MSK | LED_ORANGE_PIN_MSK | LED_RED_PIN_MSK | LED_BLUE_PIN_MSK)

/* Button Configuration ------------------------------------------------------*/
#define BUTTON_GPIO_PORT         GPIOA       /*!< PA0 - User button */
#define BUTTON_GPIO_PIN_MSK      GPIO_PIN_0
#define BUTTON_GPIO_PIN_NUM	     GPIO_PIN_0_NUM
#define BUTTON_EXTI_LINE         EXTI_Line0
#define BUTTON_IRQN              EXTI0_IRQn







// Display Configuration
#define DISPLAY_TYPE        OLED_SSD1306  // Or LCD_1602, UART, etc.
#define DISPLAY_I2C_ADDR    0x3C




/* Button Configuration (for time setting)  ------------------------------------------------------*/
#define BUTTON_GPIO_PORT        GPIOA         /*!< PA0-3 - User button */
#define BUTTON_HOUR_PIN     GPIO_PIN_0
#define BUTTON_MIN_PIN      GPIO_PIN_1
#define BUTTON_SET_PIN      GPIO_PIN_2
#define BUTTON_ALARM_PIN    GPIO_PIN_3

#define BUTTON_EXTI_LINE         EXTI_Line0
#define BUTTON_IRQN              EXTI0_IRQn
#define BUTTON_GPIO_CLK_ENABLE() do { \
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; \
} while(0)

/* Timing Configuration ------------------------------------------------------*/
#define DEBOUNCE_TIME_MS       50    /*!< Button de-bounce time */
#define LONG_PRESS_TIME_MS     1000  /*!< 2 seconds for long press */
#define DOUBLE_CLICK_MAX_MS    500   /*!< Max time between double clicks */
#define SYSTEM_TICK_MS         1     /*!< SysTick period */

/* Interrupt Priorities ------------------------------------------------------*/
#define EXTI_PRIORITY          0     /*!< Highest priority for button */
#define SYSTICK_PRIORITY       1     /*!< Medium priority for systick */

#endif
