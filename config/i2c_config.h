/**
  ******************************************************************************
  * @file    i2c_config.h
  * @brief   I2C1 Configuration for STM32F407 with 16MHz internal clock
  ******************************************************************************
  */

#ifndef I2C_CONFIG_H
#define I2C_CONFIG_H

/* I2C1 Pin Configuration (STM32F407 - Discovery Board) */
#define I2C_SCL_PORT           GPIOB
#define I2C_SCL_PIN            6U
#define I2C_SCL_AF             4U

#define I2C_SDA_PORT           GPIOB
#define I2C_SDA_PIN            7U
#define I2C_SDA_AF             4U

/* Clock Configuration - Using 16MHz internal clock (HSI) */
#define I2C_APB1_CLOCK_MHZ     16U     /* APB1 = 16MHz when system clock is 16MHz */

/* Timeout Configuration */
#define I2C_TIMEOUT_MS         100U    /* 100ms timeout */

/* Default Speed */
#define I2C_DEFAULT_SPEED      I2C_SPEED_100KHZ

#endif /* I2C_CONFIG_H */
