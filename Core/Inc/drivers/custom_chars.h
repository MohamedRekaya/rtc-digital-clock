/**
  ******************************************************************************
  * @file    custom_chars.h
  * @brief   Custom character definitions for LCD1602
  ******************************************************************************
  */

#ifndef CUSTOM_CHARS_H
#define CUSTOM_CHARS_H

#include <stdint.h>

/* Bell icon */
extern const uint8_t bell_char[8];

/* Alarm ON (filled bell) */
extern const uint8_t alarm_on_char[8];

/* Alarm OFF (empty bell) */
extern const uint8_t alarm_off_char[8];

/* Check mark (✓) */
extern const uint8_t check_char[8];

/* Cross mark (✗) */
extern const uint8_t cross_char[8];

/* Clock icon */
extern const uint8_t clock_char[8];

/* Calendar icon */
extern const uint8_t calendar_char[8];

/* Settings/gear icon */
extern const uint8_t settings_char[8];

#endif /* CUSTOM_CHARS_H */
