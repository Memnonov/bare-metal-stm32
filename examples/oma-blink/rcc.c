/*
 * rcc.c
 *
 * Reset and Clock Control
 */

#include "rcc.h"

#define BIT(x) (1UL << (x))                             // Get mask for the xth bit
#define PIN(bank, num) (((bank) - 'A') << 9) | (num))   // Find a pin somehow?
#define PINNO(pin) (pin & 255)

// -------- FLASH --------
struct flash_interface_registers {
    volatile uint32_t ACR;      // Access Control Register
    volatile uint32_t KEYR;     // Key Register
    volatile uint32_t OPTKEYR;  // Option Key Register
    volatile uint32_t SR;       // Status Register
    volatile uint32_t CR;       // Control Register
    volatile uint32_t OPTCR;    // Option Control Register
};

#define FLASH ((struct flash_interface_registers*)0x40023C00)

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

/*
 * Values for system clock of 84MHz
 */
enum PLL {
    PLL_HSI = 16,
    PLL_M = 8,
    PLL_N = 168,
    PLL_P = 1,  // Sets PLLP division 01 i.e. by 4
    PLL_Q = 7,
};

void rcc_init_system_clock(void) {
    // TODO: I2S clocks (PLLI2S)

    // Assure HSION is enabled
    RCC->CR |= BIT(0);

    while (!(RCC->CR & BIT(1))) {
        // Wait until HSI is ready (HSIRDY)
    }

    // Dísable PLL while initializing it.
    RCC->CR &= ~BIT(24);

    while (RCC->CR & BIT(25)) {
        // Wait until PPLRDY is cleared
    };

    // Select HSI as the PLLSRC clock source
    RCC->PLLCFGR &= ~BIT(22);

    // Set PLL multiplication and division
    // We should get 84 MHzu CPU and 48 MHz USB...
    // First, reset all those bits...
    RCC->PLLCFGR &= ~((0x3FU << 0) |   // M
                      (0x1FFU << 6) |  // N
                      (0x3U << 16) |   // P
                      (0xFU << 24));   // Q
    // Then set values
    RCC->PLLCFGR |= PLL_M << 0;   // M = 8
    RCC->PLLCFGR |= PLL_N << 6;   // N = 168
    RCC->PLLCFGR |= PLL_P << 16;  // P = 4
    RCC->PLLCFGR |= PLL_Q << 24;  // Q = 7

    // Adjust FLASH latency for faster CPU: for 84 we want 2 WS (3 CPU cycles)
    FLASH->ACR &= ~0xFU;            // Clear latency bits
    FLASH->ACR |= BIT(2);           // Set latency
    FLASH->ACR |= BIT(8) | BIT(9);  // Enable Instruction and Prefetch caches too...

    // Enable PLL
    RCC->CR |= BIT(24);
    while (!(RCC->CR & BIT(25))) {
        // Wait for PLL ready (PLLRDY)
    }

    // Configure Bus Clocks for 84MHz
    RCC->CFGR &= ~BIT(7);      // AHB clock division not divided -> 84Mhz (<= 100Mhz)
    RCC->CFGR &= ~BIT(15);     // APB2 high-speed prescaler not divided -> 84Mhz (<= 100Mhz)
    RCC->CFGR &= ~(7U << 10);  // Reset APB1 low-speed prescaler (APB2)
    RCC->CFGR |= (4U << 10);   // APB1 low-speed prescaler divided by 2 --> 42Mhz (<= 50 limit)

    // Switch system clock to PLL
    RCC->CFGR &= ~(3U);  // Reset
    RCC->CFGR |= 2U;     // Set SW[1:0] to 10

    while (((RCC->CFGR >> 2) & 3U) != 2U) {
        // Wait until PLL is being used as the system clock source
    }
}

void rcc_enable_gpio(uint32_t gpio_port) {
    RCC->AHB1ENR |= gpio_port;  // Set bit with mask
}
