/*
 * rcc.h
 *
 * Reset and Clock Control
 */

#ifndef RCC_H
#define RCC_H

// -------- CLOCKS & TIMERS --------

#include <stdint.h>

#define SYSTEM_CLK 84000000U
#define USB_CLK 48000000U
#define AHB_CLK 84000000U
#define APB_HIGH_SPEED_CLK 84000000U
#define APB_LOW_SPEED_CLK 42000000U

#define RCC_AHB1ENR_GPIOAEN (1U << 0)
#define RCC_AHB1ENR_GPIOBEN (1U << 1)
#define RCC_AHB1ENR_GPIOCEN (1U << 2)
#define RCC_AHB1ENR_GPIODEN (1U << 3)
#define RCC_AHB1ENR_GPIOEEN (1U << 4)
#define RCC_AHB1ENR_GPIOHEN (1U << 7)

/*
 * init_system_clock
 *
 * Initialize System Clock to run faster.
 *
 * Remember to adjust flash latency first!
 *
 * Makes the system clock use the High Speed Internal oscillator
 * and sets PLL to get a clock speed of 84 MHz.
 */
void rcc_init_system_clock(void);

/*
 * enable_gpio
 *
 * Enables a GPIO bus.
 */
void rcc_enable_gpio(uint32_t gpio_port);

#endif  // !RCC_H
