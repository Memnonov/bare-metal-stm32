
#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>

/*
 * flash_set_latency
 *
 * Sets the latency for Flash memory access.
 *
 * This is required when using higher clock speeds with PLL.
 */
void flash_set_latency(uint32_t clock_speed);

#endif  // !FLASH_H
