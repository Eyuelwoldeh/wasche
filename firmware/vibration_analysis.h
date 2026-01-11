#ifndef VIBRATION_ANALYSIS_H
#define VIBRATION_ANALYSIS_H

#include <stdint.h>
#include "adxl345.h"

// Machine states
typedef enum {
    STATE_IDLE,
    STATE_WASHING,
    STATE_SPINNING,
    STATE_DONE,
    STATE_UNKNOWN
} machine_state_t;

// Vibration analysis result
typedef struct {
    float rms_magnitude;      // Root Mean Square of vibration
    float dominant_freq;      // Dominant frequency in Hz
    machine_state_t state;    // Detected machine state
    uint32_t timestamp;       // Timestamp of analysis
} vibration_result_t;

// Function prototypes
void vibration_analysis_init(void);
void vibration_analysis_add_sample(accel_data_t *data);
bool vibration_analysis_compute(vibration_result_t *result);
machine_state_t vibration_classify_state(float rms, float freq);
float vibration_compute_rms(float *buffer, size_t length);

#endif // VIBRATION_ANALYSIS_H
