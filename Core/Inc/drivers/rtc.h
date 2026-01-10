/**
  ******************************************************************************
  * @file    rtc.h
  * @brief   Professional RTC Driver for STM32F407
  * @note    Using proper RTC hardware flags
  ******************************************************************************
  */

#ifndef RTC_H
#define RTC_H



/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdbool.h>
#include <stdint.h>

/* Configuration */
#include "rtc_config.h"

/*===================================================================
  Type Definitions
  ===================================================================*/

/**
  * @brief  Time structure (24-hour format)
  */
typedef struct {
    uint8_t hours;      /* 0-23 */
    uint8_t minutes;    /* 0-59 */
    uint8_t seconds;    /* 0-59 */
} rtc_time_t;

/**
  * @brief  Date structure
  */
typedef struct {
    uint8_t day;        /* 1-31 */
    uint8_t month;      /* 1-12 */
    uint16_t year;      /* 2000-2099 */
    uint8_t weekday;    /* 1=Monday, 7=Sunday */
} rtc_date_t;

/*===================================================================
  Alarm Types
  ===================================================================*/

/**
  * @brief  Alarm configuration structure
  */
typedef struct {
    uint8_t hour;       /* 0-23 */
    uint8_t minute;     /* 0-59 */
    uint8_t second;     /* 0-59 */
    uint8_t mask;       /* Alarm mask bits (see RTC_ALARM_MASK_xxx) */
    uint8_t weekday;    /* 1-7: Match weekday (0 = ignore) */
    bool enabled;       /* Alarm enabled flag */
} rtc_alarm_t;



/*===================================================================
  Public Functions
  ===================================================================*/

/* Core Functions --------------------------------------------------*/

/**
  * @brief  Initialize RTC if not already running
  * @retval true: Success or already running, false: Failure
  * @note   Checks INITS flag first to avoid re-initialization
  */
bool rtc_init(void);



/**
  * @brief  Check if RTC is running (INITS flag = 1)
  * @retval true: RTC is running, false: Not running
  */
bool rtc_is_clock_initialized(void);



/**
  * @brief  Set current time
  * @param  time: Pointer to time structure
  * @retval true: Success, false: Invalid time or RTC not initialized
  */
bool rtc_set_time(const rtc_time_t* time);

/**
  * @brief  Get current time
  * @param  time: Pointer to time structure (output)
  */
void rtc_get_time(rtc_time_t* time);

/**
  * @brief  Set current date
  * @param  date: Pointer to date structure
  * @retval true: Success, false: Invalid date or RTC not initialized
  */
bool rtc_set_date(const rtc_date_t* date);

/**
  * @brief  Get current date
  * @param  date: Pointer to date structure (output)
  */
void rtc_get_date(rtc_date_t* date);

/*===================================================================
  Alarm Functions
  ===================================================================*/

#if RTC_ALARM_ENABLE

/**
  * @brief  Initialize RTC alarm interrupt system
  * @retval true: Success, false: Failure
  * @note   Must be called once before using alarms
  */
bool rtc_alarm_init(void);

/**
  * @brief  Set alarm A configuration
  * @param  alarm: Pointer to alarm configuration
  * @retval true: Success, false: Failure
  */
bool rtc_set_alarm_a(const rtc_alarm_t* alarm);

/**
  * @brief  Enable alarm A
  */
void rtc_alarm_a_enable(void);

/**
  * @brief  Disable alarm A
  */
void rtc_alarm_a_disable(void);

/**
  * @brief  Check if alarm A has triggered
  * @retval true: Alarm triggered, false: Not triggered
  */
bool rtc_is_alarm_a_triggered(void);

/**
  * @brief  Clear alarm A trigger flag
  */
void rtc_clear_alarm_a(void);

/**
  * @brief  RTC Alarm interrupt handler (call from ISR)
  */
void rtc_alarm_irq_handler(void);

/**
  * @brief  Application alarm callback (weak, override in application)
  */
void rtc_alarm_callback(void);

#endif /* RTC_ALARM_ENABLE */



#endif /* RTC_H */

/******************************** END OF FILE ********************************/
