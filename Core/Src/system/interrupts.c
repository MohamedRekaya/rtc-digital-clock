/**
  ******************************************************************************
  * @file    interrupts.c
  * @brief   Interrupt handler for button EXTI.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "button.h"
#include "rtc.h"

/**
  * @brief  EXTI0 interrupt handler (PA0 button).
  */
void EXTI0_IRQHandler(void) {
    button_exti_handler();
}

#if RTC_ALARM_ENABLE

/**
  * @brief  RTC Alarm interrupt handler
  */
void RTC_Alarm_IRQHandler(void) {
    rtc_alarm_irq_handler();
}

#endif /* RTC_ALARM_ENABLE */

/**
  * @brief  RTC Wakeup interrupt handler
  */
void RTC_WKUP_IRQHandler(void) {
    rtc_wakeup_irq_handler();
}
