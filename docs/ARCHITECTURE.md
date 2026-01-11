# System Architecture

## Overview

Wasche is a distributed IoT system for monitoring laundry machines. Here's how all the pieces fit together.

## High-Level Architecture

```
┌─────────────────┐
│  Washer/Dryer   │
│   (vibrates)    │
└────────┬────────┘
         │
    ┌────▼─────┐
    │ ADXL345  │ ◄─── Accelerometer (I2C)
    │  Sensor  │
    └────┬─────┘
         │
    ┌────▼────────────┐
    │   CC2652 Node   │ ◄─── Embedded firmware (C)
    │  (End Device)   │      - Samples at 100Hz
    │                 │      - FFT analysis
    └────┬────────────┘      - State detection
         │
         │ Zigbee Mesh (2.4GHz)
         │
    ┌────▼────────────┐
    │   CC2652 Node   │ ◄─── Coordinator
    │  (Coordinator)  │      - Routes messages
    └────┬────────────┘      - Serial output
         │
         │ USB Serial
         │
    ┌────▼────────────┐
    │  Backend Server │ ◄─── Flask API (Python)
    │   (Flask/Py)    │      - Receives data
    │                 │      - Processes packets
    └────┬────────────┘
         │
         │ psycopg2
         │
    ┌────▼────────────┐
    │   PostgreSQL    │ ◄─── Database
    │    Database     │      - Stores readings
    │                 │      - Historical data
    └─────────────────┘
```

## Data Flow

### 1. Sensing (Every 10ms)

```c
// Firmware samples accelerometer
accel_data_t data;
adxl345_read_data(&data);  // Read X, Y, Z acceleration
vibration_analysis_add_sample(&data);  // Add to circular buffer
```

### 2. Analysis (Every ~1.3 seconds)

```c
// When buffer is full (128 samples)
vibration_result_t result;
vibration_analysis_compute(&result);  // Compute FFT, classify state

// Result contains:
// - RMS magnitude (vibration intensity)
// - Dominant frequency (washing vs spinning)
// - Machine state (IDLE, WASHING, SPINNING, DONE)
```

### 3. Transmission (Every 5 seconds)

```c
// Send to coordinator via Zigbee
zigbee_packet_t packet = {
    .node_id = NODE_ID,
    .machine_state = result.state,
    .rms_magnitude = result.rms,
    .dominant_freq = result.freq
};
zigbee_send_data(&packet);
```

### 4. Backend Processing

```python
# Python server receives packet via serial
packet = serial_conn.read(18)  # Read packet bytes
node_id, state, rms, freq = unpack_packet(packet)

# Store in database
INSERT INTO machine_readings (node_id, state, rms, freq)
VALUES (node_id, state, rms, freq);

# Update current status
UPDATE machine_status 
SET current_state = state, last_updated = NOW()
WHERE node_id = node_id;
```

### 5. API Access

```bash
# External clients query via REST API
GET /api/machines          # Get all machines
GET /api/machines/1        # Get specific machine
GET /api/history/1?hours=24  # Get historical data
```

## Component Details

### Firmware Layer

**Files:** `firmware/*.c`

- **main.c**: Main state machine, orchestrates everything
- **adxl345.c**: I2C driver for accelerometer
- **vibration_analysis.c**: Signal processing (FFT, RMS, classification)
- **zigbee_handler.c**: Network communication

**Key Algorithms:**

1. **FFT (Fast Fourier Transform)**: Converts time-domain vibration data to frequency domain
   - Identifies dominant frequencies
   - Helps distinguish washing (2-4Hz) from spinning (6-12Hz)

2. **RMS (Root Mean Square)**: Measures vibration intensity
   - Low RMS = idle
   - Medium RMS = washing
   - High RMS = spinning

3. **State Machine**: Handles initialization, sampling, analysis, transmission, error recovery

### Backend Layer

**Files:** `backend/server.py`

- **ZigbeeReader Thread**: Continuously reads from serial port
- **Flask API**: REST endpoints for querying data
- **Database Layer**: PostgreSQL for persistence

**Key Features:**

- Thread-safe serial port reading
- Packet validation and error handling
- Automatic status updates
- Historical data queries with time windows

### Database Layer

**Files:** `database/schema.sql`

**Tables:**

1. **nodes**: Static info about each deployed node
2. **machine_status**: Current state of each machine (updated frequently)
3. **machine_readings**: Historical data (append-only, grows over time)

**Optimizations:**

- Indexes on `(node_id, timestamp)` for fast queries
- Triggers for automatic online/offline detection
- View (`machine_info`) for convenient querying

## Network Topology

### Zigbee Mesh

```
Coordinator (0x0000)
    ├─ Node 1 (Washer A)
    ├─ Node 2 (Washer B)
    │   └─ Node 4 (Dryer B) ◄─── Routed through Node 2
    └─ Node 3 (Dryer A)
```

- Self-healing: If a node goes down, routes automatically reconfigure
- Scalable: Tested with 10 nodes, supports up to 60+
- Low latency: Sub-second message delivery

## Performance Characteristics

| Metric | Value |
|--------|-------|
| Sampling Rate | 100 Hz |
| Analysis Window | 1.28 seconds (128 samples) |
| Transmission Interval | 5 seconds |
| Network Latency | < 500ms (typical) |
| Power Consumption | 40mA active, 5μA sleep |
| Battery Life | ~24 hours (current), ~1 week (optimized) |
| Database Growth | ~17 KB/node/day |

## Security Considerations

### Current State (Not Production Ready)

- ❌ Hardcoded database password
- ❌ No authentication on API
- ❌ Unencrypted Zigbee communication
- ❌ No packet encryption

### For Production Deployment

- ✅ Use environment variables for secrets
- ✅ Add JWT authentication to API
- ✅ Enable Zigbee security (AES-128)
- ✅ Use HTTPS for API
- ✅ Implement rate limiting

## Scalability

### Current Limits
- 60 nodes per coordinator (Zigbee spec)
- ~1 MB database per node per year
- Single-threaded serial reading

### How to Scale
1. Multiple coordinators (one per building)
2. Database sharding by building/floor
3. Load balancer for API servers
4. Redis cache for current status

## Future Enhancements

1. **Frontend**: Web dashboard for visualizing machine status
2. **Notifications**: Push alerts when laundry is done
3. **Analytics**: Usage patterns, busy times, predictions
4. **Machine Learning**: Better state classification using ML
5. **Mobile App**: iOS/Android for checking on the go

## Development Timeline

- **Oct 2024**: Started project, got hardware
- **Nov 2024**: Firmware working, backend deployed
- **Dec 2024**: Real-world testing in dorms
- **Jan 2025**: System stabilized, 5 nodes deployed

## Lessons Learned

1. **Hardware is hard**: Debugging embedded systems without proper tools is painful
2. **Calibration matters**: Every machine is slightly different
3. **Real-world testing is essential**: Lab testing ≠ dorm laundry room
4. **Keep it simple**: Started with complex ML, ended with simple thresholds (works better)
5. **Documentation helps**: Especially when you forget how your own code works

---

For implementation details, see the source code and comments throughout the project.
