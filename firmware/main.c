/*
 * Wasche IoT Laundry System - Main Firmware
 * 
 * This runs on the CC2652 Zigbee nodes attached to washers/dryers
 * Samples accelerometer data, analyzes vibration patterns, sends to coordinator
 * 
 * Author: Eyuel Woldehanna
 * Date: Fall 2024
 */

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include "config.h"
#include "adxl345.h"
#include "vibration_analysis.h"
#include "zigbee_handler.h"

// State machine for main loop
typedef enum {
    STATE_INIT,
    STATE_SAMPLING,
    STATE_ANALYZING,
    STATE_TRANSMITTING,
    STATE_ERROR
} app_state_t;

static app_state_t current_state = STATE_INIT;
static uint32_t last_transmit_time = 0;
static uint32_t last_heartbeat_time = 0;

// Simple delay function (would use proper timer in production)
void delay_ms(uint32_t ms) {
    usleep(ms * 1000);
}

// Get current time in milliseconds (would use proper RTC in production)
uint32_t get_time_ms(void) {
    // TODO: implement proper timestamp using RTC or system timer
    static uint32_t fake_time = 0;
    fake_time += 10;  // increment by 10ms each call (roughly)
    return fake_time;
}

int main(void) {
    accel_data_t accel_data;
    vibration_result_t vib_result;
    
    // Initialization
    current_state = STATE_INIT;
    
    #if DEBUG_UART_ENABLE
    // Initialize UART for debugging
    // uart_init(DEBUG_BAUD_RATE);
    // uart_print("Wasche Node Starting...\n");
    #endif
    
    // Initialize accelerometer
    if (!adxl345_init()) {
        #if DEBUG_UART_ENABLE
        // uart_print("ERROR: ADXL345 init failed\n");
        #endif
        current_state = STATE_ERROR;
    }
    
    // Initialize vibration analysis
    vibration_analysis_init();
    
    // Initialize Zigbee
    if (!zigbee_init()) {
        #if DEBUG_UART_ENABLE
        // uart_print("ERROR: Zigbee init failed\n");
        #endif
        current_state = STATE_ERROR;
    }
    
    if (current_state != STATE_ERROR) {
        current_state = STATE_SAMPLING;
        #if DEBUG_UART_ENABLE
        // uart_print("Init complete. Node ID: 0x%04X\n", NODE_ID);
        #endif
    }
    
    // Main loop
    while (1) {
        uint32_t current_time = get_time_ms();
        
        switch (current_state) {
            case STATE_SAMPLING:
                // Read accelerometer data
                if (adxl345_read_data(&accel_data)) {
                    vibration_analysis_add_sample(&accel_data);
                    
                    // Check if we have enough samples to analyze
                    if (vibration_analysis_compute(&vib_result)) {
                        current_state = STATE_ANALYZING;
                    }
                } else {
                    // Read failed, maybe connection issue?
                    #if DEBUG_UART_ENABLE
                    // uart_print("WARNING: accelerometer read failed\n");
                    #endif
                }
                
                delay_ms(SAMPLE_PERIOD_MS);
                break;
                
            case STATE_ANALYZING:
                // Analysis already done by vibration_analysis_compute()
                // Just transition to transmit state
                current_state = STATE_TRANSMITTING;
                break;
                
            case STATE_TRANSMITTING:
                // Check if it's time to transmit
                if (current_time - last_transmit_time >= TRANSMIT_INTERVAL_MS) {
                    if (zigbee_send_data(&vib_result)) {
                        last_transmit_time = current_time;
                        
                        #if DEBUG_UART_ENABLE
                        // uart_print("Sent data: state=%d, rms=%.2f, freq=%.1fHz\n",
                        //           vib_result.state, vib_result.rms_magnitude, 
                        //           vib_result.dominant_freq);
                        #endif
                    } else {
                        #if DEBUG_UART_ENABLE
                        // uart_print("WARNING: transmission failed\n");
                        #endif
                    }
                }
                
                // Send heartbeat if needed
                if (current_time - last_heartbeat_time >= HEARTBEAT_INTERVAL_MS) {
                    zigbee_send_heartbeat();
                    last_heartbeat_time = current_time;
                }
                
                // Go back to sampling
                current_state = STATE_SAMPLING;
                break;
                
            case STATE_ERROR:
                // In error state, try to recover
                delay_ms(5000);  // wait 5 seconds
                
                // Try to reinit
                if (adxl345_test_connection() && zigbee_is_connected()) {
                    #if DEBUG_UART_ENABLE
                    // uart_print("Recovered from error state\n");
                    #endif
                    current_state = STATE_SAMPLING;
                }
                break;
                
            default:
                current_state = STATE_ERROR;
                break;
        }
        
        // Watchdog timer would be kicked here in production
    }
    
    return 0;  // should never reach here
}
