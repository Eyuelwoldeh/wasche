# Makefile for Wasche Firmware
# Built for TI CC2652 using SimpleLink SDK

# TODO: Set this to your SDK installation path
SIMPLELINK_SDK ?= /opt/ti/simplelink_cc13xx_cc26xx_sdk_5_40_00_40

# Compiler and tools
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

# Compiler flags
CFLAGS = -mcpu=cortex-m4 \
         -march=armv7e-m \
         -mthumb \
         -mfloat-abi=hard \
         -mfpu=fpv4-sp-d16 \
         -Wall \
         -Wextra \
         -O2 \
         -g \
         -I. \
         -I$(SIMPLELINK_SDK)/source \
         -I$(SIMPLELINK_SDK)/kernel/tirtos/packages

# Linker flags
LDFLAGS = -mcpu=cortex-m4 \
          -march=armv7e-m \
          -mthumb \
          -mfloat-abi=hard \
          -mfpu=fpv4-sp-d16 \
          -nostartfiles \
          -Wl,-T,cc2652r1f.lds \
          -lm

# Source files
SOURCES = main.c \
          adxl345.c \
          vibration_analysis.c \
          zigbee_handler.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Output file
TARGET = wasche_firmware

# Build targets
all: $(TARGET).elf $(TARGET).hex

$(TARGET).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^
	$(SIZE) $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Flash to device (using uniflash)
flash: $(TARGET).hex
	@echo "Flashing firmware..."
	# uniflash.sh -ccxml CC2652R1F.ccxml -program $(TARGET).hex

clean:
	rm -f $(OBJECTS) $(TARGET).elf $(TARGET).hex

# Dependencies
main.o: main.c config.h adxl345.h vibration_analysis.h zigbee_handler.h
adxl345.o: adxl345.c adxl345.h config.h
vibration_analysis.o: vibration_analysis.c vibration_analysis.h config.h adxl345.h
zigbee_handler.o: zigbee_handler.c zigbee_handler.h config.h vibration_analysis.h

.PHONY: all clean flash
