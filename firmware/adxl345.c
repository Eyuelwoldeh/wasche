#include "adxl345.h"
#include "config.h"
#include <ti/drivers/I2C.h>
#include <ti/drivers/GPIO.h>
#include <unistd.h>
#include <math.h>

// I2C handle - initialized externally
static I2C_Handle i2c_handle = NULL;

// Helper function to write a register
static bool write_register(uint8_t reg, uint8_t value) {
    uint8_t txBuffer[2];
    I2C_Transaction i2cTransaction;
    
    txBuffer[0] = reg;
    txBuffer[1] = value;
    
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readBuf = NULL;
    i2cTransaction.readCount = 0;
    i2cTransaction.slaveAddress = ADXL345_ADDR;
    
    return I2C_transfer(i2c_handle, &i2cTransaction);
}

// Helper function to read registers
static bool read_registers(uint8_t reg, uint8_t *buffer, size_t length) {
    I2C_Transaction i2cTransaction;
    
    // first write the register address
    i2cTransaction.writeBuf = &reg;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = buffer;
    i2cTransaction.readCount = length;
    i2cTransaction.slaveAddress = ADXL345_ADDR;
    
    return I2C_transfer(i2c_handle, &i2cTransaction);
}

bool adxl345_init(void) {
    // Open I2C
    I2C_Params i2cParams;
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;  // fast mode
    
    i2c_handle = I2C_open(0, &i2cParams);
    if (i2c_handle == NULL) {
        return false;  // rip
    }
    
    // Check device ID
    uint8_t devid;
    if (!read_registers(ADXL345_REG_DEVID, &devid, 1)) {
        return false;
    }
    
    if (devid != ADXL345_DEVICE_ID) {
        return false;  // wrong chip or connection issue
    }
    
    // Set data format: full resolution, +/- 16g range
    // full res gives us 4mg/LSB regardless of range which is nice
    if (!write_register(ADXL345_REG_DATA_FORMAT, ADXL345_FULL_RES | ADXL345_RANGE_16G)) {
        return false;
    }
    
    // Set bandwidth/output data rate to 100Hz
    // BW_RATE register: 0x0A = 100Hz
    if (!write_register(ADXL345_REG_BW_RATE, 0x0A)) {
        return false;
    }
    
    // Enable measurement mode
    if (!write_register(ADXL345_REG_POWER_CTL, ADXL345_MEASURE)) {
        return false;
    }
    
    usleep(10000);  // give it a moment to start up
    
    return true;
}

bool adxl345_read_data(accel_data_t *data) {
    uint8_t buffer[6];
    
    // Read all 6 bytes (X, Y, Z as 16-bit values)
    if (!read_registers(ADXL345_REG_DATAX0, buffer, 6)) {
        return false;
    }
    
    // Combine bytes (little-endian)
    data->x = (int16_t)((buffer[1] << 8) | buffer[0]);
    data->y = (int16_t)((buffer[3] << 8) | buffer[2]);
    data->z = (int16_t)((buffer[5] << 8) | buffer[4]);
    
    return true;
}

float adxl345_convert_to_g(int16_t raw_value) {
    // In full resolution mode, scale factor is 4 mg/LSB
    // so multiply by 0.004 to get g's
    return raw_value * 0.004f;
}

bool adxl345_test_connection(void) {
    uint8_t devid;
    if (!read_registers(ADXL345_REG_DEVID, &devid, 1)) {
        return false;
    }
    return (devid == ADXL345_DEVICE_ID);
}
