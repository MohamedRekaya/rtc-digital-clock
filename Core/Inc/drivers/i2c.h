#ifndef I2C_H
#define I2C_H

#include "stm32f4xx.h"
#include <stdbool.h>
#include <stdint.h>

/* I2C Status */
typedef enum {
    I2C_OK = 0,
    I2C_ERROR,
    I2C_BUSY,
    I2C_TIMEOUT
} i2c_status_t;

/* Public Functions */
void i2c_init(void);
i2c_status_t i2c_write_byte(uint8_t dev_addr, uint8_t data);
i2c_status_t i2c_write_bytes(uint8_t dev_addr, const uint8_t* data, uint8_t len);

#endif /* I2C_H */
