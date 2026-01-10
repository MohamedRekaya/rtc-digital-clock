/**
  ******************************************************************************
  * @file    systick.c
  * @brief   System Tick Timer (SysTick) implementation.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "systick.h"
#include "stm32f4xx.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define SYSTICK_FREQ_HZ     1000U  /* 1ms tick */

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/**
  * @brief  System tick counter (volatile - modified in ISR).
  */
static volatile uint32_t systick_counter = 0;

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  SysTick interrupt handler.
  * @note   Called every 1ms, increments systick_counter.
  * @retval None
  */
void SysTick_Handler(void) {
    systick_counter++;
}

/**
  * @brief  Initialize SysTick timer for 1ms interrupts.
  * @note   Configures SysTick for SystemCoreClock/1000 interrupts per second.
  *         Sets medium interrupt priority.
  * @retval None
  */
void systick_init(void) {
    /* Disable SysTick during configuration */
    SysTick->CTRL = 0;

    /* Calculate reload value for 1ms interrupts */
    uint32_t reload_value = (SystemCoreClock / SYSTICK_FREQ_HZ) - 1;

    /* Ensure reload value fits in 24-bit register */
    if (reload_value > 0x00FFFFFF) {
        /* Error: SystemCoreClock too high for 1ms tick */
        while(1); /* Halt on configuration error */
    }

    /* Configure SysTick */
    SysTick->LOAD = reload_value;  /* Set reload value */
    SysTick->VAL  = 0;             /* Clear current value */

    /* Enable SysTick with processor clock and interrupt */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |  /* Use processor clock */
                    SysTick_CTRL_TICKINT_Msk   |  /* Enable interrupt */
                    SysTick_CTRL_ENABLE_Msk;      /* Enable counter */

    /* Set medium priority (0 = highest, 15 = lowest on Cortex-M4) */
    NVIC_SetPriority(SysTick_IRQn, SYSTICK_PRIORITY);
}

/**
 * @brief  Get current system time in milliseconds.
 * @note   Safe to call from both main code and interrupts.
 *         On 32-bit MCUs, reading a uint32_t is atomic, so
 *         we do NOT need to disable interrupts to get a consistent value.
 *         Disabling interrupts is unnecessary and could hurt real-time performance.
 * @retval Milliseconds since SysTick initialization.
 */
uint32_t systick_get_ticks(void) {
    return systick_counter;
}


/**
  * @brief  Check if a time delay has elapsed (non-blocking).
  * @param  last_tick: Time stamp from systick_get_ticks().
  * @param  delay_ms: Delay duration in milliseconds.
  * @retval 1 if delay has elapsed, 0 otherwise.
  * @note   Handles 32-bit counter overflow correctly.
  */
bool systick_delay_elapsed(uint32_t last_tick, uint32_t delay_ms) {
    return (systick_get_ticks() - last_tick) >= delay_ms;
}

/**
  * @brief  Busy-wait delay (blocking).
  * @param  delay_ms: Delay duration in milliseconds.
  * @retval None
  * @note   Uses systick_delay_elapsed() internally.
  */
void systick_delay_ms(uint32_t delay_ms) {
    uint32_t start_tick = systick_get_ticks();
    while (!systick_delay_elapsed(start_tick, delay_ms)) {
        /* Wait - CPU spins here */
        __NOP(); /* Optional: prevent compiler optimization */
    }
}



