# Toolchain
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

# Directories
SRC_DIR   = src
BUILD_DIR = build
INC_DIR   = include

# Files
TARGET = firmware

# Flags
CFLAGS  = -W -Wall -Wextra -Werror -Wundef -Wshadow -Wdouble-promotion \
          -Wformat-truncation -fno-common -Wconversion \
          -g3 -O0 -ffunction-sections -fdata-sections \
          -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
          -I$(INC_DIR) -MMD -MP

LDFLAGS = -T f411.ld -nostartfiles -nostdlib --specs=nano.specs \
          -lc -lgcc -Wl,--gc-sections \
          -Wl,-Map=$(BUILD_DIR)/$(TARGET).elf.map

# Sources / objects
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJS    = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
DEPS    = $(OBJS:.o=.d)

# Default target
all: $(BUILD_DIR)/$(TARGET).bin

# Link
$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

# Binary
$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# Compile
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) -c $< $(CFLAGS) -o $@

# Flash
flash: $(BUILD_DIR)/$(TARGET).bin
	st-flash --reset write $< 0x08000000

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Dependencies
-include $(DEPS)

.PHONY: all clean flash
