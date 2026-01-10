/**
  ******************************************************************************
  * @file    rtc_config.h
  * @brief   RTC Configuration for STM32F407
  * @note    To use the RTC_CLOCK_SOURCE_LSE you need to provide an external 32k clock
  ******************************************************************************
  */

#ifndef RTC_CONFIG_H
#define RTC_CONFIG_H

/*===================================================================
  RTC Clock Source Selection
  ===================================================================*/
#define RTC_CLOCK_SOURCE_LSE     1   /* External 32.768 kHz crystal */
#define RTC_CLOCK_SOURCE_LSI     2   /* Internal ~32 kHz RC oscillator */

/* Select your clock source here: */
#define RTC_SOURCE               RTC_CLOCK_SOURCE_LSI  /* LSI always works */

/*===================================================================
  Prescaler Configuration
  ===================================================================*/
/* For LSI (~32 kHz): */
#define RTC_LSI_ASYNC_PRESCALER      127
#define RTC_LSI_SYNC_PRESCALER       255     /* (127+1)*(255+1) = 32768 */

/* For LSE (32.768 kHz): */
#define RTC_LSE_ASYNC_PRESCALER      127
#define RTC_LSE_SYNC_PRESCALER       255     /* (127+1)*(255+1) = 32768 */

/*===================================================================
  Time Format
  ===================================================================*/
#define RTC_HOUR_FORMAT_12       0
#define RTC_HOUR_FORMAT_24       1

#define RTC_TIME_FORMAT          RTC_HOUR_FORMAT_24

/*===================================================================
  Backup Register Usage (Optional - for alarm persistence)
  ===================================================================*/
#define BKP_REG_ALARM_HOUR       1   /* RTC_BKP1R: Alarm hour */
#define BKP_REG_ALARM_MINUTE     2   /* RTC_BKP2R: Alarm minute */
#define BKP_REG_ALARM_ENABLED    3   /* RTC_BKP3R: Alarm enabled flag */

/* Only for alarm persistence, not for RTC initialization check */
#define ALARM_ENABLED_MAGIC      0xCCCC

/*===================================================================
  Timeout Values (in cycles)
  ===================================================================*/
#define LSE_STARTUP_TIMEOUT      3000000     /* ~2 seconds */
#define LSI_STARTUP_TIMEOUT      100000      /* ~100 ms */
#define RTC_INIT_TIMEOUT         10000
#define RTC_SYNC_TIMEOUT         10000

/*===================================================================
  Alarm Configuration
  ===================================================================*/
/* Enable/disable alarm features */
#define RTC_ALARM_ENABLE            1  /* Master enable for alarms */

/* Alarm A Configuration */
#define RTC_ALARM_A_ENABLE          1
#define RTC_ALARM_A_TIMEOUT         100000  /* Timeout for alarm write */

/* Interrupt Configuration */
#define RTC_ALARM_EXTI_LINE         17      /* EXTI line 17 for RTC Alarm */
#define RTC_ALARM_IRQn              RTC_Alarm_IRQn
#define RTC_ALARM_IRQ_PRIORITY      5       /* NVIC priority (0-15) */

/* Default alarm mask (what to match) */
#define RTC_ALARM_MASK_SECONDS      0x01    /* Bit 0: Match seconds */
#define RTC_ALARM_MASK_MINUTES      0x02    /* Bit 1: Match minutes */
#define RTC_ALARM_MASK_HOURS        0x04    /* Bit 2: Match hours */
#define RTC_ALARM_MASK_DATE         0x08    /* Bit 3: Match date/weekday */

/* Common alarm masks */
#define RTC_ALARM_MASK_HH_MM        0x09    /* Match hour and minute only */
#define RTC_ALARM_MASK_HH_MM_SS     0x08    /* Match hour, minute, second */
#define RTC_ALARM_MASK_ALL          0x0F    /* Match everything */

#endif /* RTC_CONFIG_H */
