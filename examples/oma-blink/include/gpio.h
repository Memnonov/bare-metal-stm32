#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

// Struct in the shape of a single GPIO port in memory
struct gpio {
    volatile uint32_t MODER;    // Mode Register                    : 0x00
    volatile uint32_t OTYPER;   // Output Type Register             : 0x04
    volatile uint32_t OSPEEDR;  // Output Speer Register            : 0x08
    volatile uint32_t PUPDR;    // Pull-Up Pull-Down Register       : 0x0C
    volatile uint32_t IDR;      // Input Data Register              : 0x10
    volatile uint32_t ODR;      // Output Data Register             : 0x14
    volatile uint32_t BSRR;     // Bit Set/Reset Register           : 0x18
    volatile uint32_t LCKR;     // Configuration Lock Register      : 0x1C
    volatile uint32_t AFRL;     // Alternate Function Low Register  : 0x20
    volatile uint32_t AFRH;     // Alternate Function High Register : 0x24
};

// -------- GPIO registers --------

// GPIO Ports
#define GPIOA ((struct gpio*)0x40020000)
#define GPIOB ((struct gpio*)0x40020400)
#define GPIOC ((struct gpio*)0x40020800)
#define GPIOD ((struct gpio*)0x40020C00)
#define GPIOE ((struct gpio*)0x40021000)
#define GPIOH ((struct gpio*)0x40021C00)

#endif  // !GPIO_H
