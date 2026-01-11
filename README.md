# Wasche IoT Laundry System 🧺

> Because waiting in the laundry room to check if the washer's done is so 2010

## What's This?

Wasche is an IoT system I built to solve a real problem in my dorm - nobody knows when the washers/dryers are available. This system uses accelerometers to detect vibration patterns from washers and dryers, then sends that data through a Zigbee mesh network to a central server. You can check machine status from your phone instead of walking down 4 flights of stairs.

**Tech Stack:**
- C firmware for CC2652 Zigbee microcontroller
- ADXL345 accelerometer for vibration detection
- Python backend (Flask)
- PostgreSQL database
- Zigbee mesh networking (60+ node capacity)

## Architecture

```
[Washer/Dryer] -> [ADXL345] -> [CC2652 Zigbee Node] -> [Zigbee Coordinator] -> [Python Backend] -> [PostgreSQL DB]
```

The system uses a self-healing mesh topology with sub-second latency. Each machine has its own node that samples vibration data at 100Hz and uses FFT analysis to detect different machine states (idle, washing, spinning, done).

## Project Structure

```
wasche/
├── firmware/           # C code for CC2652 microcontroller
├── backend/           # Python Flask server
├── database/          # PostgreSQL schemas and migrations
├── config/            # Configuration files
└── docs/              # Documentation and notes
```

## Getting Started

### Hardware Requirements
- CC2652 Zigbee module (I used the TI LAUNCHXL-CC26X2R1)
- ADXL345 accelerometer breakout board
- 3.3V power supply

### Software Setup

1. **Clone the repo:**
```bash
git clone https://github.com/Eyuelwoldeh/wasche.git
cd wasche
```

2. **Set up the database:**
```bash
cd database
psql -U postgres -f schema.sql
```

3. **Install Python dependencies:**
```bash
cd backend
pip install -r requirements.txt
```

4. **Flash the firmware:**
```bash
cd firmware
# Follow instructions in firmware/README.md
```

5. **Run the backend:**
```bash
cd backend
python server.py
```

## Current Status

✅ Firmware for single-node vibration detection  
✅ ADXL345 driver with I2C communication  
✅ Zigbee mesh networking (coordinator + end devices)  
✅ Python backend API  
✅ PostgreSQL database schema  
🚧 Web dashboard (coming soon)  
🚧 Mobile app (in planning)  

## Features

- **Real-time vibration monitoring** - samples at 100Hz
- **Machine state detection** - idle, running, spinning, done
- **Zigbee mesh network** - supports 60+ nodes with self-healing
- **Low latency** - sub-second updates from nodes to server
- **Historical data** - track machine usage patterns

## Technical Deep Dive

### Vibration Pattern Detection

The firmware samples the ADXL345 at 100Hz and uses a simple FFT to analyze frequency components. Different machine states have different vibration signatures:
- Idle: < 0.1g RMS
- Washing: 0.3-0.8g RMS, ~2-4Hz dominant frequency
- Spinning: 1.5-3.0g RMS, ~6-12Hz dominant frequency

### Zigbee Mesh Topology

I implemented a tree-based mesh where each node can route messages. The coordinator handles ~60 nodes with message queuing and acknowledgment. Network self-heals by finding alternate routes if a node drops.

## Why I Built This

Honestly, I got tired of walking to the basement only to find all washers taken. Plus I wanted to learn embedded systems and Zigbee networking. This project forced me to deal with:
- Low-level C programming for microcontrollers
- I2C protocol and sensor interfacing
- Wireless mesh networking
- Real-time data processing
- Full-stack integration

## License

MIT - do whatever you want with it

## Contact

Eyuel Woldehanna - woldeh1@stolaf.edu

Questions? Found a bug? Hit me up!
