#include "vibration_analysis.h"
#include "config.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

// Circular buffer for storing samples
static float sample_buffer_x[BUFFER_SIZE];
static float sample_buffer_y[BUFFER_SIZE];
static float sample_buffer_z[BUFFER_SIZE];
static uint16_t sample_index = 0;
static uint16_t samples_collected = 0;

// Simple FFT implementation (Cooley-Tukey algorithm)
// yeah I know I could use a library but wanted to understand it
typedef struct {
    float real;
    float imag;
} complex_t;

// Bit reversal for FFT
static uint16_t reverse_bits(uint16_t n, uint16_t bits) {
    uint16_t reversed = 0;
    for (int i = 0; i < bits; i++) {
        if (n & (1 << i)) {
            reversed |= 1 << (bits - 1 - i);
        }
    }
    return reversed;
}

// In-place FFT (recursive approach was too memory intensive)
static void compute_fft(complex_t *data, uint16_t n) {
    // bit reversal permutation
    uint16_t bits = 0;
    uint16_t temp = n;
    while (temp > 1) {
        bits++;
        temp >>= 1;
    }
    
    for (uint16_t i = 0; i < n; i++) {
        uint16_t j = reverse_bits(i, bits);
        if (j > i) {
            complex_t tmp = data[i];
            data[i] = data[j];
            data[j] = tmp;
        }
    }
    
    // FFT computation
    for (uint16_t size = 2; size <= n; size *= 2) {
        float angle = -2.0f * M_PI / size;
        complex_t wlen = {cosf(angle), sinf(angle)};
        
        for (uint16_t i = 0; i < n; i += size) {
            complex_t w = {1.0f, 0.0f};
            
            for (uint16_t j = 0; j < size / 2; j++) {
                complex_t u = data[i + j];
                complex_t v = {
                    w.real * data[i + j + size/2].real - w.imag * data[i + j + size/2].imag,
                    w.real * data[i + j + size/2].imag + w.imag * data[i + j + size/2].real
                };
                
                data[i + j].real = u.real + v.real;
                data[i + j].imag = u.imag + v.imag;
                data[i + j + size/2].real = u.real - v.real;
                data[i + j + size/2].imag = u.imag - v.imag;
                
                // multiply w by wlen
                float tmp_real = w.real * wlen.real - w.imag * wlen.imag;
                w.imag = w.real * wlen.imag + w.imag * wlen.real;
                w.real = tmp_real;
            }
        }
    }
}

void vibration_analysis_init(void) {
    memset(sample_buffer_x, 0, sizeof(sample_buffer_x));
    memset(sample_buffer_y, 0, sizeof(sample_buffer_y));
    memset(sample_buffer_z, 0, sizeof(sample_buffer_z));
    sample_index = 0;
    samples_collected = 0;
}

void vibration_analysis_add_sample(accel_data_t *data) {
    // Convert to g's and store in circular buffer
    sample_buffer_x[sample_index] = adxl345_convert_to_g(data->x);
    sample_buffer_y[sample_index] = adxl345_convert_to_g(data->y);
    sample_buffer_z[sample_index] = adxl345_convert_to_g(data->z);
    
    sample_index = (sample_index + 1) % BUFFER_SIZE;
    
    if (samples_collected < BUFFER_SIZE) {
        samples_collected++;
    }
}

float vibration_compute_rms(float *buffer, size_t length) {
    float sum = 0.0f;
    for (size_t i = 0; i < length; i++) {
        sum += buffer[i] * buffer[i];
    }
    return sqrtf(sum / length);
}

bool vibration_analysis_compute(vibration_result_t *result) {
    // Need full buffer to do analysis
    if (samples_collected < BUFFER_SIZE) {
        return false;
    }
    
    // Compute magnitude for each sample (sqrt(x^2 + y^2 + z^2))
    float magnitude[BUFFER_SIZE];
    for (int i = 0; i < BUFFER_SIZE; i++) {
        magnitude[i] = sqrtf(
            sample_buffer_x[i] * sample_buffer_x[i] +
            sample_buffer_y[i] * sample_buffer_y[i] +
            sample_buffer_z[i] * sample_buffer_z[i]
        );
    }
    
    // Compute RMS
    result->rms_magnitude = vibration_compute_rms(magnitude, BUFFER_SIZE);
    
    // Prepare data for FFT
    complex_t fft_data[BUFFER_SIZE];
    for (int i = 0; i < BUFFER_SIZE; i++) {
        fft_data[i].real = magnitude[i];
        fft_data[i].imag = 0.0f;
    }
    
    // Perform FFT
    compute_fft(fft_data, BUFFER_SIZE);
    
    // Find dominant frequency (skip DC component at index 0)
    float max_magnitude = 0.0f;
    uint16_t max_index = 1;
    
    for (int i = 1; i < BUFFER_SIZE / 2; i++) {  // only need first half
        float mag = sqrtf(fft_data[i].real * fft_data[i].real + 
                         fft_data[i].imag * fft_data[i].imag);
        if (mag > max_magnitude) {
            max_magnitude = mag;
            max_index = i;
        }
    }
    
    // Convert bin index to frequency
    result->dominant_freq = (float)max_index * SAMPLE_RATE_HZ / BUFFER_SIZE;
    
    // Classify machine state
    result->state = vibration_classify_state(result->rms_magnitude, result->dominant_freq);
    
    // TODO: add actual timestamp
    result->timestamp = 0;
    
    return true;
}

machine_state_t vibration_classify_state(float rms, float freq) {
    // Based on empirical testing (will need to tune these)
    
    if (rms < IDLE_THRESHOLD) {
        return STATE_IDLE;
    }
    
    if (rms >= SPINNING_MIN) {
        // High vibration = spinning cycle
        return STATE_SPINNING;
    }
    
    if (rms >= WASHING_MIN && rms <= WASHING_MAX) {
        // Medium vibration with lower frequency = washing
        if (freq >= 2.0f && freq <= 4.0f) {
            return STATE_WASHING;
        }
    }
    
    // If we had high vibration before and now it's low, machine is done
    // TODO: implement state tracking to detect done state properly
    
    return STATE_UNKNOWN;
}
