#ifndef ADXL345_H
#define ADXL345_H

#include <stdint.h>
#include <stdbool.h>

// ADXL345 Register Map
#define ADXL345_REG_DEVID 0x00
#define ADXL345_REG_POWER_CTL 0x2D
#define ADXL345_REG_DATA_FORMAT 0x31
#define ADXL345_REG_BW_RATE 0x2C
#define ADXL345_REG_DATAX0 0x32
#define ADXL345_REG_DATAY0 0x34
#define ADXL345_REG_DATAZ0 0x36

// Power Control bits
#define ADXL345_MEASURE (1 << 3)

// Data format bits
#define ADXL345_FULL_RES (1 << 3)
#define ADXL345_RANGE_16G 0x03

// Device ID
#define ADXL345_DEVICE_ID 0xE5

// Acceleration data structure
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} accel_data_t;

// Function prototypes
bool adxl345_init(void);
bool adxl345_read_data(accel_data_t *data);
float adxl345_convert_to_g(int16_t raw_value);
bool adxl345_test_connection(void);

#endif // ADXL345_H
