/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   STM32F4xx RTC Driver - Using proper hardware flags
  * @note    Tested on STM32F407 MCU(Discovery Board) and STM32F401 MCU(Nucleo Board)
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "rtc.h"
#include <string.h>

/* Private Macros ------------------------------------------------------------*/

/* RTC Status Flags */
#define RTC_IS_INITIALIZED()     ((RTC->ISR & RTC_ISR_INITS) != 0)
#define RTC_IS_IN_INIT_MODE()    ((RTC->ISR & RTC_ISR_INITF) != 0)
#define RTC_IS_SYNCHRONIZED()    ((RTC->ISR & RTC_ISR_RSF) != 0)



/* Private Function Prototypes -----------------------------------------------*/

static bool backup_domain_init(void);
static void backup_domain_reset(void);
static bool clock_source_init(void);
static bool rtc_clock_init(void);
static void rtc_write_protection_disable(void);
static void rtc_write_protection_enable(void);
static bool rtc_enter_init_mode(void);
static bool rtc_exit_init_mode(void);
static uint8_t bcd_to_bin(uint8_t bcd);
static uint8_t bin_to_bcd(uint8_t bin);
static bool validate_time(const rtc_time_t* time);
static bool validate_date(const rtc_date_t* date);
static bool rtc_wait_for_sync(uint32_t timeout);


/* Public Functions ----------------------------------------------------------*/

/**
  * @brief  Initialize RTC if not already running
  * @retval true: Success or already running, false: Failure
  */
bool rtc_init(void) {


    /* Step 1: Initialize backup domain */
    if (!backup_domain_init()) {
        return false;
    }

    /* RESET BACKUP DOMAIN */
    backup_domain_reset();

    /* Step 2: Initialize clock source */
    if (!clock_source_init()) {
        return false;
    }

    /* Step 3: Initialize RTC clock */
    if (!rtc_clock_init()) {
        return false;
    }

    /* Step 4: Wait for RTC to be ready */
    for (volatile uint32_t i = 0; i < 1000; i++) {
        if (RTC_IS_INITIALIZED()) {
            break;
        }
    }

    /* Step 5: If not initialized (INITS = 0), configure prescalers */
    if (!RTC_IS_INITIALIZED()) {
        /* Enter initialization mode */
        if (!rtc_enter_init_mode()) {
            return false;
        }

        /* Configure prescalers */
        #if RTC_SOURCE == RTC_CLOCK_SOURCE_LSI
            RTC->PRER = (RTC_LSI_ASYNC_PRESCALER << 16) | RTC_LSI_SYNC_PRESCALER;
        #elif RTC_SOURCE == RTC_CLOCK_SOURCE_LSE
            RTC->PRER = (RTC_LSE_ASYNC_PRESCALER << 16) | RTC_LSE_SYNC_PRESCALER;
        #endif

        /* Configure hour format */
        #if RTC_TIME_FORMAT == RTC_HOUR_FORMAT_24
            RTC->CR &= ~RTC_CR_FMT;
        #else
            RTC->CR |= RTC_CR_FMT;
        #endif

        /* Exit initialization mode */
        if (!rtc_exit_init_mode()) {

            return false;
        }
    }

    /* Step 6: Wait for registers to sync */
    if (!rtc_wait_for_sync(RTC_SYNC_TIMEOUT)) {

        return false;
    }

    /* Step 7: If date not set (INITS = 0), set default date */
    if (!RTC_IS_INITIALIZED()) {
        /* Set default date: 2024-01-01 Monday */
        rtc_date_t default_date = {
            .day = 1,
            .month = 1,
            .year = 2024,  /* This makes INITS = 1 */
            .weekday = 2   /* Monday */
        };

        if (!rtc_set_date(&default_date)) {
            return false;
        }

        /* Set default time: 00:00:00 */
        rtc_time_t default_time = {
            .hours = 0,
            .minutes = 0,
            .seconds = 0
        };

        if (!rtc_set_time(&default_time)) {
            return false;
        }
    }

    return true;
}

/**
  * @brief  Set current time
  */
bool rtc_set_time(const rtc_time_t* time) {
    if (!validate_time(time)) {

        return false;
    }


    /* Wait for sync */
    if (!rtc_wait_for_sync(RTC_SYNC_TIMEOUT)) {

        return false;
    }

    /* Enter initialization mode */
    if (!rtc_enter_init_mode()) {

        return false;
    }

    /* Convert and set time */
    uint32_t tr = 0;
    tr |= (bin_to_bcd(time->seconds) << 0);
    tr |= (bin_to_bcd(time->minutes) << 8);
    tr |= (bin_to_bcd(time->hours) << 16);

    /* Handle 12-hour format if needed */
    #if RTC_TIME_FORMAT == RTC_HOUR_FORMAT_12
        if (time->hours >= 12) {
            tr |= (1 << 22);  /* PM bit */
            if (time->hours > 12) {
                tr |= (bin_to_bcd(time->hours - 12) << 16);
            }
        }
    #endif

    RTC->TR = tr;

    /* Exit initialization mode */
    if (!rtc_exit_init_mode()) {

        return false;
    }

    return true;
}

/**
  * @brief  Get current time
  */
void rtc_get_time(rtc_time_t* time) {
    if (time == NULL || !RTC_IS_INITIALIZED()) {
        return;
    }

    /* Wait for sync */
    rtc_wait_for_sync(RTC_SYNC_TIMEOUT);

    /* Read time register */
    uint32_t tr = RTC->TR;

    time->seconds = bcd_to_bin((tr >> 0) & 0x7F);
    time->minutes = bcd_to_bin((tr >> 8) & 0x7F);

    #if RTC_TIME_FORMAT == RTC_HOUR_FORMAT_24
        time->hours = bcd_to_bin((tr >> 16) & 0x3F);
    #else
        /* 12-hour format */
        uint8_t hour_bcd = (tr >> 16) & 0x1F;
        uint8_t hour = bcd_to_bin(hour_bcd);

        if (tr & (1 << 22)) {  /* PM bit */
            if (hour != 12) hour += 12;
        } else {  /* AM */
            if (hour == 12) hour = 0;
        }
        time->hours = hour;
    #endif
}

/**
  * @brief  Set current date
  */
bool rtc_set_date(const rtc_date_t* date) {
    if (!validate_date(date)) {
        return false;
    }

    /* Check if RTC is initialized
    if (!rtc_is_clock_initialized()) {
        last_error = RTC_ERROR_NONE;
        return false;
    }
    */

    /* Wait for sync */
    if (!rtc_wait_for_sync(RTC_SYNC_TIMEOUT)) {
        return false;
    }

    /* Enter initialization mode */
    if (!rtc_enter_init_mode()) {
        return false;
    }

    /* Convert and set date */
    uint32_t dr = 0;
    dr |= (bin_to_bcd(date->day) << 0);
    dr |= (bin_to_bcd(date->month) << 8);
    dr |= (bin_to_bcd(date->year % 100) << 16);
    dr |= ((date->weekday % 7) << 13);

    RTC->DR = dr;

    /* Exit initialization mode */
    if (!rtc_exit_init_mode()) {
        return false;
    }

    return true;
}

/**
  * @brief  Get current date
  */
void rtc_get_date(rtc_date_t* date) {
    if (date == NULL || !RTC_IS_INITIALIZED() ) {
        return;
    }

    /* Wait for sync */
    rtc_wait_for_sync(RTC_SYNC_TIMEOUT);

    /* Read date register */
    uint32_t dr = RTC->DR;

    date->day = bcd_to_bin((dr >> 0) & 0x3F);
    date->month = bcd_to_bin((dr >> 8) & 0x1F);
    date->year = 2000 + bcd_to_bin((dr >> 16) & 0xFF);
    date->weekday = (dr >> 13) & 0x07;
}

/**
  * @brief  Wait for RTC registers synchronization
  */
bool rtc_wait_for_sync(uint32_t timeout) {
	RTC->ISR &= ~RTC_ISR_RSF;  /* Clear flag for next read */

    while (!RTC_IS_SYNCHRONIZED()) {
        if (timeout-- == 0) {
            return false;
        }
    }

    return true;
}

/* Private Functions ---------------------------------------------------------*/

/**
  * @brief  Initialize backup domain
  */
static bool backup_domain_init(void) {
    /* Enable PWR clock */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;

    /* Enable backup domain access */
    PWR->CR |= PWR_CR_DBP;

    /* Wait for backup domain access */
    uint32_t timeout = 100000;
    while ((PWR->CR & PWR_CR_DBP) == 0) {
        if (timeout-- == 0) {
            return false;
        }
    }

    return true;
}

/**
  * @brief  Reset backup domain (sets/clears BDRST bit)
  * @note   Minimal version - assumes backup domain is already accessible
  */
static void backup_domain_reset(void) {
    RCC->BDCR |= RCC_BDCR_BDRST;    /* Set reset */
    for(volatile int i=0; i<1000; i++);  /* Short delay */
    RCC->BDCR &= ~RCC_BDCR_BDRST;   /* Clear reset */
    for(volatile int i=0; i<1000; i++);  /* Recovery delay */
}

/**
  * @brief  Initialize clock source
  */
/**
  * @brief  Initialize clock source
  */
static bool clock_source_init(void) {
    #if RTC_SOURCE == RTC_CLOCK_SOURCE_LSI
        /* Enable LSI */
        RCC->CSR |= RCC_CSR_LSION;

        /* Wait for LSI ready */
        uint32_t timeout = LSI_STARTUP_TIMEOUT;
        while ((RCC->CSR & RCC_CSR_LSIRDY) == 0) {
            if (timeout-- == 0) {
                return false;
            }
        }
        return true;

    #elif RTC_SOURCE == RTC_CLOCK_SOURCE_LSE
        /* First disable LSE if it was enabled */
        RCC->BDCR &= ~RCC_BDCR_LSEON;



        /* Clear LSE bypass (using crystal, not external clock) */
        RCC->BDCR &= ~RCC_BDCR_LSEBYP;

        /* Enable LSE */
        RCC->BDCR |= RCC_BDCR_LSEON;

        /* IMPORTANT: Add a small delay before checking LSERDY */
        for (volatile uint32_t i = 0; i < 10000; i++);

        /* Wait for LSE ready */
        uint32_t timeout = LSE_STARTUP_TIMEOUT;
        while ((RCC->BDCR & RCC_BDCR_LSERDY) == 0) {
            if (timeout-- == 0) {
                /* Try with bypass mode (external clock) if crystal doesn't work */
                RCC->BDCR &= ~RCC_BDCR_LSEON;
                for (volatile uint32_t i = 0; i < 1000; i++);
                RCC->BDCR |= RCC_BDCR_LSEBYP;  /* Enable bypass */
                RCC->BDCR |= RCC_BDCR_LSEON;

                timeout = LSE_STARTUP_TIMEOUT;
                while ((RCC->BDCR & RCC_BDCR_LSERDY) == 0) {
                    if (timeout-- == 0) {
                        RCC->BDCR &= ~RCC_BDCR_LSEON;
                        return false;
                    }
                }
            }
        }
        return true;

    #else
        return false;
    #endif
}

/**
  * @brief  Initialize RTC clock
  */
static bool rtc_clock_init(void) {


    /* Select RTC clock source */
    RCC->BDCR &= ~RCC_BDCR_RTCSEL;

    #if RTC_SOURCE == RTC_CLOCK_SOURCE_LSI
        RCC->BDCR |= RCC_BDCR_RTCSEL_1;  /* LSI */
    #elif RTC_SOURCE == RTC_CLOCK_SOURCE_LSE
        RCC->BDCR |= RCC_BDCR_RTCSEL_0;  /* LSE */
    #endif

    /* Enable RTC clock */
    RCC->BDCR |= RCC_BDCR_RTCEN;

    /* Small delay for clock to stabilize */
    for (volatile uint32_t i = 0; i < 1000; i++);

    return true;
}

/**
  * @brief  Disable write protection
  */
static void rtc_write_protection_disable(void) {
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
}

/**
  * @brief  Enable write protection
  */
static void rtc_write_protection_enable(void) {
    RTC->WPR = 0xFF;
}

/**
  * @brief  Enter initialization mode safely
  */
static bool rtc_enter_init_mode(void) {
    rtc_write_protection_disable();

    RTC->ISR |= RTC_ISR_INIT;

    uint32_t timeout = RTC_INIT_TIMEOUT;
    while (!RTC_IS_IN_INIT_MODE()) {
        if (timeout-- == 0) {
            rtc_write_protection_enable();
            return false;
        }
    }

    return true;
}

/**
  * @brief  Exit initialization mode safely
  */
static bool rtc_exit_init_mode(void) {
    RTC->ISR &= ~RTC_ISR_INIT;
    rtc_write_protection_enable();

    /* Verify we exited init mode */
    uint32_t timeout = RTC_INIT_TIMEOUT;
    while (RTC_IS_IN_INIT_MODE()) {
        if (timeout-- == 0) {
            return false;
        }
    }

    return true;
}

/**
  * @brief  Convert binary to BCD
  */
static uint8_t bin_to_bcd(uint8_t bin) {
    if (bin >= 100) return 0;
    return ((bin / 10) << 4) | (bin % 10);
}

/**
  * @brief  Convert BCD to binary
  */
static uint8_t bcd_to_bin(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

/**
  * @brief  Validate time
  */
static bool validate_time(const rtc_time_t* time) {
    if (time == NULL) return false;
    if (time->hours >= 24) return false;
    if (time->minutes >= 60) return false;
    if (time->seconds >= 60) return false;
    return true;
}

/**
  * @brief  Validate date
  */
static bool validate_date(const rtc_date_t* date) {
    if (date == NULL) return false;
    if (date->month < 1 || date->month > 12) return false;
    if (date->day < 1 || date->day > 31) return false;
    if (date->year < 2000 || date->year > 2099) return false;
    if (date->weekday < 1 || date->weekday > 7) return false;
    return true;
}


