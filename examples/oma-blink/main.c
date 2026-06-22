/**
 * Minimal bare-metal STM32 example
 */

#include <stdint.h>

#define BIT(x) (1UL << (x))                             // Get mask for the xth bit
#define PIN(bank, num) (((bank) - 'A') << 9) | (num))   // Find a pin somehow?
#define PINNO(pin) (pin & 255)

// -------- RESET AND CLOCK CONTROL --------

// Impose a struct in the shape of the memory mapping of the peripherals for typed access.
struct rcc {
    volatile uint32_t CR;            // Clock control register                   : 0x00
    volatile uint32_t PLLCFGR;       // PLL configuration register               : 0x04
    volatile uint32_t CFGR;          // Clock configuration register             : 0x08
    volatile uint32_t CIR;           // Clock interrupt register                 : 0x0C
    volatile uint32_t AHB1RSTR;      // AHB1 peripheral reset register           : 0x10
    volatile uint32_t AHB2RSTR;      // AHB2 peripheral reset register           : 0x14
    volatile uint32_t RESERVED0[2];  // Reserved                                 : 0x18 - 0x1C

    volatile uint32_t APB1RSTR;      // APB1 peripheral reset register           : 0x20
    volatile uint32_t APB2RSTR;      // APB2 peripheral reset register           : 0x24
    volatile uint32_t RESERVED1[2];  // Reserved                                 : 0x28 - 0x2C

    volatile uint32_t AHB1ENR;       // AHB1 peripheral clock enable register    : 0x30
    volatile uint32_t AHB2ENR;       // AHB2 peripheral clock enable register    : 0x34
    volatile uint32_t RESERVED2[2];  // Reserved                                 : 0x38 - 0x3C

    volatile uint32_t APB1ENR;       // APB1 peripheral clock enable register    : 0x40
    volatile uint32_t APB2ENR;       // APB2 peripheral clock enable register    : 0x44
    volatile uint32_t RESERVED3[2];  // Reserved                                 : 0x48 - 0x4C

    volatile uint32_t AHB1LPENR;  // AHB1 low-power enable register              : 0x50
    volatile uint32_t AHB2LPENR;  // AHB2 low-power enable register              : 0x54
    volatile uint32_t RESERVED4;  // Reserved                                    : 0x58 - 0x5C

    volatile uint32_t APB1LPENR;     // APB1 low-power enable register           : 0x60
    volatile uint32_t APB2LPENR;     // APB2 low-power enable register           : 0x64
    volatile uint32_t RESERVED5[2];  // Reserved                                 : 0x68 - 0x6C

    volatile uint32_t BDCR;          // Backup domain control register           : 0x70
    volatile uint32_t CSR;           // Clock control & status register          : 0x74
    volatile uint32_t RESERVED6[2];  // Reserved                                 : 0x78 - 0x7C

    volatile uint32_t SSCGR;       // Spread spectrum clock generation register  : 0x80
    volatile uint32_t PLLI2SCFGR;  // PLLI2S configuration register              : 0x84
    volatile uint32_t RESERVED7;   // Reserved                                   : 0x88
    volatile uint32_t DCKCFGR;     // Domain Clock Configuration Register         : 0x8C
};

// Treat address 0x40023800 like the defined rcc struct.
#define RCC ((struct rcc*)0x40023800)

// -----------------------------------------

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

// GPIO Ports
#define GPIOA ((struct gpio*)0x40020000)
#define GPIOB ((struct gpio*)0x40020400)
#define GPIOC ((struct gpio*)0x40020800)
#define GPIOD ((struct gpio*)0x40020C00)
#define GPIOE ((struct gpio*)0x40021000)
#define GPIOH ((struct gpio*)0x40021C00)

// -------- GPIO registers --------

// -----------------------------------------

int main(void) {
    // Get that LED on
    RCC->AHB1ENR |= (1U << 0);         // Set first bit to enable GPIOAEN
    GPIOA->MODER &= ~(3U << (5 * 2));  // 11 Mask to reset PA5 mode to 00
    GPIOA->MODER |= (1U << (5 * 2));   // Set PA5 mode to 01 (GP Output Mode)
    GPIOA->ODR |= (1U << 5);           // Set corresponding output pint to HIGH

    uint32_t cnt = 0, half;
    while (1) {
        cnt += 2;
        half = cnt / 2;
        ++half;
        if (half == 123) {
            cnt = 0;
        }
    }
}

// Startup code
__attribute__((naked, noreturn)) void _reset(void) {
    extern long _sbss, _ebss, _sdata, _edata, _sidata;

    for (long* dst = &_sbss; dst < &_ebss; dst++) *dst = 0;

    for (long *dst = &_sdata, *src = &_sidata; dst < &_edata;) *dst++ = *src++;

    main();

    for (;;) (void)0;  // Infinite loop - should never be reached
}

extern void _estack(void);  // Defined in link.ld

// 16 standard and 91 STM32-specific handlers
__attribute__((section(".vectors"))) void (*const tab[16 + 91])(void) = {_estack, _reset};  // More interrupt handlers go here eventually?
