#include "flash.h"

#include <stdint.h>

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

void flash_set_latency(uint32_t clock_speed_mhz) {
    uint32_t ws;  // wait states

    switch (clock_speed_mhz) {
        case 0 ... 29:
            ws = 0;
            break;
        case 30 ... 64:
            ws = 1;
            break;
        case 65 ... 90:
            ws = 2;
            break;
        case 91 ... 100:
            ws = 2;
            break;
        default:
            ws = 3;  // You might have a problem!
            break;
    }

    FLASH->ACR &= ~0xFU;  // Clear latency bits
    FLASH->ACR |= ws;
    FLASH->ACR |= 7U << 8;  // Enable Data, Instruction and Prefetch Caches

    while ((FLASH->ACR & 0xF) != ws) {
        // Wait until latency is set
    }
}
