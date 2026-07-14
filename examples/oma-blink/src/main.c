/**
 * Minimal bare-metal STM32 example
 */

#include <stdbool.h>
#include <stdint.h>

#include "flash.h"
#include "rcc.h"
#include "gpio.h"

#define BIT(x) (1UL << (x))                             // Get mask for the xth bit
#define PIN(bank, num) (((bank) - 'A') << 9) | (num))   // Find a pin somehow?
#define PINNO(pin) (pin & 255)

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

// -------- FUNCTION PROTOTYPES --------

int main(void);
static void init_led(void);
static void init_system_tick(uint32_t);
// static void init_system_clock(void);
static void spin(uint32_t);

static void _default_handler(void);
static void _systick_handler(void);

// -------- STARTUP CODE --------
__attribute__((naked, noreturn)) void _reset(void) {
    extern long _sbss, _ebss, _sdata, _edata, _sidata;

    for (long* dst = &_sbss; dst < &_ebss; dst++) {
        *dst = 0;
    }

    for (long *dst = &_sdata, *src = &_sidata; dst < &_edata;) {
        *dst++ = *src++;
    }

    main();

    while (true) (void)0;  // Infinite loop - should never be reached
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

#define SYSTEM_CLK 84000000U
#define USB_CLK 48000000U
#define AHB_CLK 84000000U
#define APB_HIGH_SPEED_CLK 84000000U
#define APB_LOW_SPEED_CLK 42000000U

/*
 * Sets system clock to 84MHz
 */
enum PLL {
    PLL_HSI = 16,
    PLL_M = 8,
    PLL_N = 168,
    PLL_P = 1,  // Sets PLLP division 01 i.e. by 4
    PLL_Q = 7,
};

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
    const uint32_t CLOCK_84_MHZ = 84000000;
    const uint32_t BLINK_INTERVAL = 500;

    flash_set_latency(CLOCK_84_MHZ / 1000000);
    rcc_init_system_clock();
    rcc_enable_gpio(RCC_AHB1ENR_GPIOAEN);
    init_system_tick(SYSTEM_CLK / 1000);  // This gives us a 1ms SysTick

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

// -------- HELPERS ------------------------------------

/*
 * Initializes STM32F411RE Nucleo boards built-in user led.
 *
 * GPIOA must be enabled.
 * Set GPIO Pin A5 to GP output mode.
 *
 */
static void init_led(void) {
    // Get that LED on
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
