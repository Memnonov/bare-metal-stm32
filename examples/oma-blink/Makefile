CFLAGS  ?=  -W -Wall -Wextra -Werror -Wundef -Wshadow -Wdouble-promotion \
	-Wformat-truncation -fno-common -Wconversion \
	-g3 -O0 -ffunction-sections -fdata-sections -I. \
	-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
	-MMD -MP $(EXTRA_CFLAGS)

LDFLAGS ?= -T f411.ld -nostartfiles -nostdlib --specs=nano.specs \
	-lc -lgcc -Wl,--gc-sections -Wl,-Map=$@.map

SOURCES = main.c rcc.c
OBJS    = $(SOURCES:.c=.o)
DEPS    = $(OBJS:.o=.d)

all: firmware.bin

firmware.elf: $(OBJS)
	arm-none-eabi-gcc $(OBJS) $(LDFLAGS) -o $@

firmware.bin: firmware.elf
	arm-none-eabi-objcopy -O binary $< $@

%.o: %.c
	arm-none-eabi-gcc -c $< $(CFLAGS) -o $@

flash: firmware.bin
	st-flash --reset write $< 0x08000000

clean:
	rm -rf *.o *.d firmware.*

-include $(DEPS)

.PHONY: all clean flash
