#include "lcd1602_i2c.h"
#include "i2c.h"
#include <string.h>

/* Private Variables */
static uint8_t backlight_state = LCD_BACKLIGHT;

/* Private Functions */
static void delay_us(uint32_t us) {
    /* Simple delay - adjust based on your clock speed */
    volatile uint32_t count = us * 16;  /* For 16MHz */
    while (count--);
}

static void lcd_send_nibble(uint8_t data, uint8_t rs) {
    uint8_t byte = data | rs | backlight_state;

    /* Send with E=1 */
    i2c_write_byte(LCD_I2C_ADDR, byte | LCD_E);
    delay_us(10);

    /* Send with E=0 */
    i2c_write_byte(LCD_I2C_ADDR, byte);
    delay_us(10);
}

static void lcd_send_byte(uint8_t data, uint8_t rs) {
    /* Send high nibble */
    lcd_send_nibble(data & 0xF0, rs);

    /* Send low nibble */
    lcd_send_nibble((data << 4) & 0xF0, rs);
}

/* Public Functions */
void lcd_init(void) {
    /* Wait for LCD power-up */
    delay_us(50000);

    /* Initialization sequence for 4-bit mode */
    lcd_send_nibble(0x30, 0);  /* Function set: 8-bit */
    delay_us(4500);
    lcd_send_nibble(0x30, 0);  /* Function set: 8-bit */
    delay_us(150);
    lcd_send_nibble(0x30, 0);  /* Function set: 8-bit */
    delay_us(150);
    lcd_send_nibble(0x20, 0);  /* Function set: 4-bit */
    delay_us(150);

    /* Now we can use lcd_send_byte for 4-bit mode */
    lcd_send_byte(0x28, 0);    /* Function set: 4-bit, 2-line, 5x8 dots */
    lcd_send_byte(0x0C, 0);    /* Display ON, cursor OFF, blink OFF */
    lcd_send_byte(0x06, 0);    /* Entry mode: increment, no shift */
    lcd_send_byte(0x01, 0);    /* Clear display */
    delay_us(2000);
}

void lcd_clear(void) {
    lcd_send_byte(0x01, 0);
    delay_us(2000);
}

void lcd_home(void) {
    lcd_send_byte(0x02, 0);
    delay_us(2000);
}

void lcd_set_cursor(uint8_t row, uint8_t col) {
    uint8_t address;

    if (row == 0) {
        address = 0x00 + col;
    } else {
        address = 0x40 + col;
    }

    lcd_send_byte(0x80 | address, 0);
}

void lcd_write_char(char c) {
    lcd_send_byte(c, LCD_RS);
}

void lcd_write_string(const char* str) {
    while (*str) {
        lcd_write_char(*str++);
    }
}

void lcd_backlight_on(void) {
    backlight_state = LCD_BACKLIGHT;
    i2c_write_byte(LCD_I2C_ADDR, backlight_state);
}

void lcd_backlight_off(void) {
    backlight_state = 0;
    i2c_write_byte(LCD_I2C_ADDR, backlight_state);
}



/**
  * @brief  Create custom character in LCD CGRAM
  * @param  location: CGRAM address (0-7)
  * @param  charmap: 8-byte array (5x8 pixels each)
  */
void lcd_create_char(uint8_t location, const uint8_t charmap[8]) {
    if (location > 7) return;  /* Only 8 custom chars available */

    /* Set CGRAM address: 0x40 + (location * 8) */
    lcd_send_byte(0x40 | (location << 3), 0);

    /* Send 8 bytes of character data */
    for (uint8_t i = 0; i < 8; i++) {
        lcd_send_byte(charmap[i], 1);
    }

    /* Return to DDRAM (display RAM) */
    lcd_send_byte(0x80, 0);
}

/**
  * @brief  Write custom character to current cursor position
  * @param  location: CGRAM address (0-7)
  */
void lcd_write_custom_char(uint8_t location) {
    if (location > 7) return;
    lcd_send_byte(location, 1);
}
