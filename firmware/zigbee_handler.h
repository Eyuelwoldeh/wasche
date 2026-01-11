#ifndef ZIGBEE_HANDLER_H
#define ZIGBEE_HANDLER_H

#include <stdint.h>
#include <stdbool.h>
#include "vibration_analysis.h"

// Zigbee packet types
#define PKT_TYPE_DATA 0x01
#define PKT_TYPE_HEARTBEAT 0x02
#define PKT_TYPE_ACK 0x03

// Packet structure for sending vibration data
typedef struct __attribute__((packed)) {
    uint8_t packet_type;
    uint16_t node_id;
    uint8_t machine_state;
    float rms_magnitude;
    float dominant_freq;
    uint32_t timestamp;
    uint16_t checksum;
} zigbee_packet_t;

// Function prototypes
bool zigbee_init(void);
bool zigbee_send_data(vibration_result_t *result);
bool zigbee_send_heartbeat(void);
bool zigbee_is_connected(void);
uint16_t zigbee_compute_checksum(uint8_t *data, size_t length);

#endif // ZIGBEE_HANDLER_H
