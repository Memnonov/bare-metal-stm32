/**
 * Minimal bare-metal STM32 example
 */

#include <stdbool.h>
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

// -------- SYSTICK AND TIMERS ----

/*
 * System timer register in the SCS
 */
struct syst {
    volatile uint32_t CSR;    // Control and Status Register
    volatile uint32_t RVR;    // Reload Value Register
    volatile uint32_t CVR;    // Current Value Register
    volatile uint32_t CALIB;  // Calibration Value Registerr
};

// Set SYST to begin at 0xE000E010
#define SYSTICK ((struct syst*)0xE000E010)

/*
 * A timer register map for TIMX X = {2, 5}
 *
 * Only use with general-purpose timers TIM2 and TIM5!
 */
struct timx {
    volatile uint32_t CR1;       // Control  1
    volatile uint32_t CR2;       // Control  2
    volatile uint32_t SMCR;      // Slave Mode Control
    volatile uint32_t DIER;      // DMA/Interrupt Enable
    volatile uint32_t SR;        // Status
    volatile uint32_t EGR;       // Event Generation
    volatile uint32_t CCMR1;     // Capture/Compare Mode 1
    volatile uint32_t CCMR2;     // Capture/Compare Mode 2
    volatile uint32_t CCER;      // Capture/Compare Enable
    volatile uint32_t CNT;       // Counter
    volatile uint32_t PSC;       // Prescaler
    volatile uint32_t ARR;       // Auto-Reload
    volatile uint32_t CCR1;      // Capture/Compare 1
    volatile uint32_t CCR2;      // Capture/Compare 2
    volatile uint32_t CCR3;      // Capture/Compare 3
    volatile uint32_t CCR4;      // Capture/Compare 4
    volatile uint32_t RESERVED;  // Reserved
    volatile uint32_t DCR;       // DMA Control
    volatile uint32_t DMAR;      // DMA Address for full transfer
    volatile uint32_t OR;        // Option Register (TIM2 & TIM5)
};

// -------- GPIO registers --------

// GPIO Ports
#define GPIOA ((struct gpio*)0x40020000)
#define GPIOB ((struct gpio*)0x40020400)
#define GPIOC ((struct gpio*)0x40020800)
#define GPIOD ((struct gpio*)0x40020C00)
#define GPIOE ((struct gpio*)0x40021000)
#define GPIOH ((struct gpio*)0x40021C00)

// -------- FUNCTION PROTOTYPES --------

int main(void);
static void init_led(void);
static void init_system_tick(uint32_t);
static void init_sysclk(void);
static void spin(uint32_t);

static void _default_handler(void);
static void _systick_handler(void);

// -------- STARTUP CODE --------
__attribute__((naked, noreturn)) void _reset(void) {
    extern long _sbss, _ebss, _sdata, _edata, _sidata;

    for (long* dst = &_sbss; dst < &_ebss; dst++) *dst = 0;

    for (long *dst = &_sdata, *src = &_sidata; dst < &_edata;) *dst++ = *src++;

    main();

    for (;;) (void)0;  // Infinite loop - should never be reached
}

// -------- VECTOR TABLE HANDLERS --------

/*
 * Trap execution in a loop in case of an unexpected exception.
 */
static void _default_handler(void) {
    while (true) {
    }
}

/*
 * System Ticks
 *
 * Static global counter for System Timer ticks.
 */
static volatile uint32_t s_ticks = 0;

static void _systick_handler(void) { ++s_ticks; }

// -------- VECTOR TABLE --------

extern void _estack(void);  // Defined in link.ld

// 16 standard and 91 STM32-specific handlers
__attribute__((section(".vectors"))) void (*const tab[16 + 91])(void) = {
    _estack,           // 0: Stack pointer begin
    _reset,            // 1: Reset function,
    _default_handler,  // 2: NMI
    _default_handler,  // 3: HardFault
    _default_handler,  // 4: MemManage
    _default_handler,  // 5: BusFault
    _default_handler,  // 6: UsageFault
    _default_handler,  // 7: Reserverd
    _default_handler,  // 8: Reserverd
    _default_handler,  // 9: Reserverd
    _default_handler,  // 10: Reserved
    _default_handler,  // 11: SVCall
    _default_handler,  // 12: DebugMonitor
    _default_handler,  // 13: Reserved
    _default_handler,  // 14: PendSV
    _systick_handler,  // 15: SysTick
                       // And more...
};

// -------- CLOCKS & TIMERS --------

/*
 * Initialize System Clock to run faster.
 *
 * Sets the system clock to use the High Speed Internal oscillator
 * and sets it to 84 Mhz.
 */
static void init_sysclk(void) {
    // Dísable PLL while initializing it.
    RCC->CR &= ~BIT(24);

    // Ajust FLASH latency for faster CPU: for 86 we want 2 WS (3 CPU cycles)
    // ADJUST IT

    while (RCC->CR & BIT(25)) {
        // Wait until PPLRDY is cleared
    };

    // Set PLLM to 8 (0b11) so that HSI/PLL = 16/8 = 2 MHz
    // M = 8
    // N = 168
    // P = 4
    // Q = 7
    // and we should get 84 MHzu CPU and 48 MHz USB...
    // also remember to adjust FLASH delay whatever?

    // Enable HSI with HSION bit
    RCC->CR |= BIT(0);

    // Check if HSI is ready (HSIRDY bit)
    // bool hsi_rdy = (RCC->CR & (1U << 1)) != 0;
    // Or wait for it
    while (!(RCC->CR & BIT(1))) {
        // Wait for HSIRDY
    }

    // Enable PLL after initializing it.
    RCC->CR |= BIT(24);
}

/*
 * This let's us the System Timer instead of some lame loops.
 * Parameter ticks determines timer frequency.
 */
static void init_system_tick(uint32_t ticks) {
    SYSTICK->RVR = ticks - 1;                  // Set reload value to ticks
    SYSTICK->CVR = 0;                          // Clear Current Value Register
    SYSTICK->CSR |= BIT(0) | BIT(1) | BIT(2);  // Enable SysTick and use processor clock

    // This not actually needed...
    // RCC->APB2ENR |= (1U << 14);  // Enable System Configuration Controller Clock
}

// -------- MAIN ------------------------------------
int main(void) {
    // const uint32_t DELAY = 500000;
    const uint32_t CLOCK_16_MHZ = 16000000;  // 16 Mhz clock
    const uint32_t BLINK_INTERVAL = 500;

    init_system_tick(CLOCK_16_MHZ / 1000);  // This gives us a 1ms SysTick

    init_led();

    // Init counters for blinks
    uint32_t now = 0;
    uint32_t next_blink = now + BLINK_INTERVAL;

    // Main loop
    for (;;) {
        // GPIOA->ODR ^= (1U << 5);  // XOR PA5 bit to toggle led
        // spin(DELAY);
        now = s_ticks;

        if (now >= next_blink) {
            GPIOA->ODR ^= (1U << 5);  // XOR PA5 bit to toggle led
            next_blink += BLINK_INTERVAL;
        }
    }
}

/*
 * Enable clock for AHB1 bus and set Pin A5 to GP output mode.
 * This is to use the STM32F411RE Nucleo boards built-in user led.
 */
static void init_led(void) {
    // Get that LED on
    RCC->AHB1ENR |= (1U << 0);         // Set first bit to enable GPIOAEN
    GPIOA->MODER &= ~(3U << (5 * 2));  // 11 Mask to reset PA5 mode to 00
    GPIOA->MODER |= (1U << (5 * 2));   // Set PA5 mode to 01 (GP Output Mode)
}

/*
 * Clumsy blocking sleep.
 */
static inline void spin(volatile uint32_t delay) {
    while (delay--) {
    }
}
