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

/**
  * @brief  Alarm configuration
  */
typedef struct {
    uint8_t hour;       /* 0-23 */
    uint8_t minute;     /* 0-59 */
    uint8_t second;     /* 0-59 */
    bool enabled;       /* Alarm enabled */
    uint8_t mask;       /* Alarm mask bits:
                         * bit0: Seconds match
                         * bit1: Minutes match
                         * bit2: Hours match
                         * bit3: Date/Weekday match
                         */
    bool weekday_match; /* true: match weekday, false: match date */
    uint8_t weekday;    /* 1-7 when weekday_match = true */
} rtc_alarm_t;

/**
  * @brief  RTC status using hardware flags
  */
typedef enum {
    RTC_STATUS_UNKNOWN = 0,         /* Can't determine status */
    RTC_STATUS_NOT_INITIALIZED,     /* Backup domain not accessible */
    RTC_STATUS_INIT_MODE,           /* INITF flag set (in init mode) */
    RTC_STATUS_RUNNING,             /* RSF flag set (registers synced) */
    RTC_STATUS_ERROR                /* Timeout or other error */
} rtc_status_t;

/**
  * @brief  RTC error codes
  */
typedef enum {
    RTC_ERROR_NONE = 0,
    RTC_ERROR_CLOCK_FAILED,
    RTC_ERROR_INIT_TIMEOUT,
    RTC_ERROR_SYNC_TIMEOUT,
    RTC_ERROR_INVALID_TIME,
    RTC_ERROR_INVALID_DATE,
    RTC_ERROR_BACKUP_DOMAIN,
    RTC_ERROR_ALREADY_RUNNING
} rtc_error_t;

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
  * @brief  Force re-initialization of RTC (even if already running)
  * @retval true: Success, false: Failure
  * @note   Use with caution - resets RTC time
  */
bool rtc_force_init(void);

/**
  * @brief  Get current RTC status using hardware flags
  * @retval RTC status based on INITS, RSF, and INITF flags
  */
rtc_status_t rtc_get_status(void);

/**
  * @brief  Check if RTC is running (INITS flag = 1)
  * @retval true: RTC is running, false: Not running
  */
bool rtc_is_clock_initialized(void);

/**
  * @brief  Get last error code
  * @retval Error code
  */
rtc_error_t rtc_get_last_error(void);

/* Time/Date Functions ---------------------------------------------*/

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

/* [Rest of the functions remain the same...] */



#endif /* RTC_H */

/******************************** END OF FILE ********************************/
