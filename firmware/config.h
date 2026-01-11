#ifndef CONFIG_H
#define CONFIG_H

// Wasche Configuration File
// TODO: move this to a separate config file that's not in git (has node IDs and stuff)

// Device Configuration
#define NODE_ID 0x0001  // change this for each device
#define DEVICE_TYPE_END_DEVICE  // or DEVICE_TYPE_COORDINATOR

// Sampling Configuration
#define SAMPLE_RATE_HZ 100
#define SAMPLE_PERIOD_MS (1000 / SAMPLE_RATE_HZ)
#define BUFFER_SIZE 128  // power of 2 for FFT

// Vibration Thresholds (in g's)
#define IDLE_THRESHOLD 0.1
#define WASHING_MIN 0.3
#define WASHING_MAX 0.8
#define SPINNING_MIN 1.5

// Zigbee Network Config
#define ZIGBEE_CHANNEL 15  // 2.4GHz channel (11-26)
#define ZIGBEE_PAN_ID 0xFACE  // lol
#define COORDINATOR_ADDR 0x0000

// I2C Configuration
#define I2C_CLOCK_SPEED 400000  // 400kHz
#define ADXL345_ADDR 0x53  // default I2C address

// Timing
#define TRANSMIT_INTERVAL_MS 5000  // send data every 5 seconds
#define HEARTBEAT_INTERVAL_MS 30000  // 30 sec keepalive

// Debug
#define DEBUG_UART_ENABLE 1
#define DEBUG_BAUD_RATE 115200

#endif // CONFIG_H
