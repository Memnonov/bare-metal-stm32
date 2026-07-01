STM32F411 clock bring-up TODO (bare metal, PLL → SYSCLK)

Goal: move from HSI → PLL → stable system clock (e.g. 84 MHz) safely.

1. Decide clock source
 Use HSI (16 MHz) or HSE (external crystal, if present)
For Nucleo default: assume HSI
2. Enable base oscillator
 Turn on HSI or HSE in RCC->CR
 Wait for ready flag (HSIRDY / HSERDY)
3. Configure Flash for higher speed
 Set FLASH->ACR latency (critical)
~84 MHz → 2 wait states
 Enable prefetch/cache if desired (optional but common)
4. Configure PLL
 Disable PLL first
 Set:
PLLM (input divider)
PLLN (multiplier)
PLLP (SYSCLK divider)
PLLQ (USB if needed)
 Select PLL source (HSI or HSE)
 Enable PLL
 Wait for PLLRDY
5. Switch system clock
 Set RCC->CFGR.SW = PLL
 Wait until SWS confirms PLL is active
6. Set bus prescalers
 AHB = /1
 APB1 = /4 (important: max 42 MHz domain)
 APB2 = /2 or /1 depending setup
7. Verify
 Check RCC->CFGR SWS bits
 Blink timing sanity check (SysTick or timer)
 Confirm no HardFault on switch
8. Optional sanity tools
 SysTick calibration check (detect wrong clock)
 GDB watch RCC->CFGR, RCC->CR
 Toggle GPIO in timer interrupt instead of busy loop
Common failure points
Flash latency set too low → instant HardFault
PLL not ready before switch
Wrong PLLP encoding
APB1 overclock (>42 MHz on F411)
