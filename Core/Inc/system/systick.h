#ifndef _SYSTICK_H_
#define _SYSTICK_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macros -----------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Initialize SysTick timer for 1ms interrupts.
  * @note   Must be called before any other systick function.
  * @retval None
  */
void systick_init(void);


/**
  * @brief  Get current system time in milliseconds.
  * @note   Counts from 0 to 2^32-1 (~49.7 days) then overflows.
  * @retval Milliseconds since systick initialization.
  */
uint32_t systick_get_ticks(void);

/**
  * @brief  Check if a time delay has elapsed (non-blocking).
  * @param  last_tick: Time stamp from systick_get_ticks().
  * @param  delay_ms: Delay duration in milliseconds.
  * @retval 1 if delay has elapsed, 0 otherwise.
  */
bool systick_delay_elapsed(uint32_t last_tick, uint32_t delay_ms);


/**
  * @brief  Busy-wait delay (blocking).
  * @note   Use sparingly - prevents other processing.
  * @param  delay_ms: Delay duration in milliseconds.
  * @retval None
  */
void systick_delay_ms(uint32_t delay_ms);















#endif /* _SYSTICK_H_ */
