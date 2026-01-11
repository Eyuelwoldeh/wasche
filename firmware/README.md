# Firmware for CC2652 Zigbee Module

This directory contains the embedded C firmware for the Wasche IoT nodes.

## What's Here

- `main.c` - Main application loop, handles initialization and state machine
- `adxl345.c/h` - Driver for ADXL345 accelerometer (I2C communication)
- `zigbee_handler.c/h` - Zigbee networking layer (mesh routing, packet handling)
- `vibration_analysis.c/h` - Signal processing for vibration pattern detection
- `Makefile` - Build configuration for CC2652

## Hardware Setup

Connect ADXL345 to CC2652:
- SDA -> DIO5 (I2C Data)
- SCL -> DIO6 (I2C Clock)
- VCC -> 3.3V
- GND -> GND

## Building the Firmware

You'll need the TI SimpleLink SDK installed. I used version 5.40.

```bash
# Set SDK path (adjust to your installation)
export SIMPLELINK_SDK=/path/to/simplelink_cc13xx_cc26xx_sdk_5_40_00_40

# Build
make clean
make

# Flash to device
make flash
```

## Configuration

Edit `config.h` to set:
- Node ID (unique per device)
- Sampling rate (default 100Hz)
- Vibration thresholds
- Zigbee network settings

## Notes

- The firmware uses TI-RTOS for task scheduling
- I2C runs at 400kHz (fast mode)
- Zigbee coordinator must be running before end devices join
- Power consumption: ~40mA active, ~5μA sleep mode (haven't fully optimized yet)

## Debugging

Use Code Composer Studio or connect a UART adapter to DIO3 (TX) for debug output.

Baud rate: 115200
