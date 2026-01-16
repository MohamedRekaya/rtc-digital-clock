#ifndef LCD1602_I2C_H
#define LCD1602_I2C_H

#include <stdint.h>

/* LCD I2C Address (7-bit) */
#define LCD_I2C_ADDR    0x27

/* PCF8574 Pin Mapping for LCD */
#define LCD_RS          (1 << 0)  /* Register Select */
#define LCD_RW          (1 << 1)  /* Read/Write */
#define LCD_E           (1 << 2)  /* Enable */
#define LCD_BACKLIGHT   (1 << 3)  /* Backlight control */
#define LCD_D4          (1 << 4)
#define LCD_D5          (1 << 5)
#define LCD_D6          (1 << 6)
#define LCD_D7          (1 << 7)

/* Public Functions */
void lcd_init(void);
void lcd_clear(void);
void lcd_home(void);
void lcd_set_cursor(uint8_t row, uint8_t col);
void lcd_write_char(char c);
void lcd_write_string(const char* str);
void lcd_backlight_on(void);
void lcd_backlight_off(void);

// Add these to your existing lcd1602_i2c.h:

/* Custom Character Locations (CGRAM addresses 0-7) */
#define LCD_CUSTOM_BELL      0
#define LCD_CUSTOM_ALARM_ON  1
#define LCD_CUSTOM_ALARM_OFF 2
#define LCD_CUSTOM_CHECK     3
#define LCD_CUSTOM_CROSS     4
#define LCD_CUSTOM_CLOCK     5
#define LCD_CUSTOM_CALENDAR  6
#define LCD_CUSTOM_SETTINGS  7

/* Public Function Declarations */
void lcd_create_char(uint8_t location, const uint8_t charmap[8]);
void lcd_write_custom_char(uint8_t location);

#endif /* LCD1602_I2C_H */
