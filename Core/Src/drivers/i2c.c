#include "i2c.h"

/* Private Variables */
static volatile bool i2c_busy = false;
static volatile i2c_status_t i2c_last_error = I2C_OK;

/* Private Function */
static void i2c_wait_busy_free(void) {
    uint32_t timeout = 100000;
    while (I2C1->SR2 & I2C_SR2_BUSY) {
        if (timeout-- == 0) {
            i2c_last_error = I2C_TIMEOUT;
            break;
        }
    }
}

/**
  * @brief  Initialize I2C1 peripheral
  */
void i2c_init(void) {
    /* Enable GPIOB clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    /* Enable I2C1 clock */
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    /* Configure PB6 (SCL) and PB7 (SDA) as Alternate Function Open-Drain */
    GPIOB->MODER &= ~((3 << 12) | (3 << 14));     /* Clear MODER bits */
    GPIOB->MODER |= ((2 << 12) | (2 << 14));      /* Alternate function */

    GPIOB->OTYPER |= (1 << 6) | (1 << 7);         /* Open-drain */
    GPIOB->OSPEEDR |= ((3 << 12) | (3 << 14));    /* High speed */
    GPIOB->PUPDR &= ~((3 << 12) | (3 << 14));
    GPIOB->PUPDR |= ((1 << 12) | (1 << 14));      /* Pull-up */

    GPIOB->AFR[0] |= (4 << (6 * 4)) | (4 << (7 * 4)); /* AF4 = I2C1 */

    /* Disable I2C before configuration */
    I2C1->CR1 &= ~I2C_CR1_PE;

    /* Configure for 16MHz APB1 clock, 100kHz */
    I2C1->CR2 = 16;          /* 16 MHz */
    I2C1->CCR = 80;          /* 100kHz: 16MHz/(2*100kHz) = 80 */
    I2C1->TRISE = 17;        /* 16 + 1 = 17 */

    /* Enable ACK and peripheral */
    I2C1->CR1 |= I2C_CR1_ACK | I2C_CR1_PE;

    i2c_busy = false;
    i2c_last_error = I2C_OK;
}

/**
  * @brief  Write single byte to I2C device
  * @param  dev_addr: 7-bit device address
  * @param  data: Byte to send
  * @retval i2c_status_t: Status of operation
  */
i2c_status_t i2c_write_byte(uint8_t dev_addr, uint8_t data) {
    uint32_t timeout = 100000;

    if (i2c_busy) return I2C_BUSY;
    i2c_busy = true;
    i2c_last_error = I2C_OK;

    /* Wait if bus is busy */
    i2c_wait_busy_free();
    if (i2c_last_error == I2C_TIMEOUT) {
        i2c_busy = false;
        return I2C_TIMEOUT;
    }

    /* Generate START condition */
    I2C1->CR1 |= I2C_CR1_START;

    /* Wait for SB flag */
    timeout = 100000;
    while (!(I2C1->SR1 & I2C_SR1_SB)) {
        if (timeout-- == 0) {
            I2C1->CR1 |= I2C_CR1_STOP;
            i2c_busy = false;
            i2c_last_error = I2C_TIMEOUT;
            return I2C_TIMEOUT;
        }
    }

    /* Send device address (write mode) */
    I2C1->DR = dev_addr << 1;  /* LSB = 0 for write */

    /* Wait for ADDR flag */
    timeout = 100000;
    while (!(I2C1->SR1 & I2C_SR1_ADDR)) {
        if (I2C1->SR1 & I2C_SR1_AF) {
            /* NACK received */
            I2C1->SR1 &= ~I2C_SR1_AF;
            I2C1->CR1 |= I2C_CR1_STOP;
            i2c_busy = false;
            i2c_last_error = I2C_ERROR;
            return I2C_ERROR;
        }
        if (timeout-- == 0) {
            I2C1->CR1 |= I2C_CR1_STOP;
            i2c_busy = false;
            i2c_last_error = I2C_TIMEOUT;
            return I2C_TIMEOUT;
        }
    }

    /* Clear ADDR flag (read SR1 then SR2) */
    volatile uint32_t tmp;
    tmp = I2C1->SR1;
    tmp = I2C1->SR2;
    (void)tmp;

    /* Wait for TXE flag and send data */
    timeout = 100000;
    while (!(I2C1->SR1 & I2C_SR1_TXE)) {
        if (timeout-- == 0) {
            I2C1->CR1 |= I2C_CR1_STOP;
            i2c_busy = false;
            i2c_last_error = I2C_TIMEOUT;
            return I2C_TIMEOUT;
        }
    }
    I2C1->DR = data;

    /* Wait for BTF flag (transfer complete) */
    timeout = 100000;
    while (!(I2C1->SR1 & I2C_SR1_BTF)) {
        if (timeout-- == 0) {
            I2C1->CR1 |= I2C_CR1_STOP;
            i2c_busy = false;
            i2c_last_error = I2C_TIMEOUT;
            return I2C_TIMEOUT;
        }
    }

    /* Generate STOP condition */
    I2C1->CR1 |= I2C_CR1_STOP;

    /* Wait for STOP to clear */
    timeout = 100000;
    while (I2C1->CR1 & I2C_CR1_STOP) {
        if (timeout-- == 0) {
            i2c_busy = false;
            i2c_last_error = I2C_TIMEOUT;
            return I2C_TIMEOUT;
        }
    }

    i2c_busy = false;
    return I2C_OK;
}

/**
  * @brief  Write multiple bytes to I2C device
  * @param  dev_addr: 7-bit device address
  * @param  data: Pointer to data buffer
  * @param  len: Number of bytes to send
  * @retval i2c_status_t: Status of operation
  */
i2c_status_t i2c_write_bytes(uint8_t dev_addr, const uint8_t* data, uint8_t len) {
    i2c_status_t status;

    if (len == 0) return I2C_OK;

    for (uint8_t i = 0; i < len; i++) {
        status = i2c_write_byte(dev_addr, data[i]);
        if (status != I2C_OK) {
            return status;
        }
    }

    return I2C_OK;
}
