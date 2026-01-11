# Hardware Setup Guide

## Parts List

### Per Node:
- TI CC2652R1 Zigbee Module (I used LAUNCHXL-CC26X2R1 dev board)
- ADXL345 Accelerometer breakout board
- 3.3V power supply or battery pack
- Jumper wires
- Enclosure (optional but recommended)

### Coordinator:
- One CC2652R1 configured as coordinator
- USB-to-Serial adapter for connecting to server
- Raspberry Pi or any Linux machine for backend

## Wiring

### ADXL345 to CC2652

```
ADXL345          CC2652
-------          ------
VCC    ------>   3.3V
GND    ------>   GND
SDA    ------>   DIO5 (I2C Data)
SCL    ------>   DIO6 (I2C Clock)
```

### Power Considerations

The CC2652 + ADXL345 draws about 40mA during active operation. Options:
- Wall adapter (easiest)
- USB power bank (portable)
- 3x AA batteries (lasts ~24 hours with current code)

I haven't optimized for power yet - nodes could sleep between samples to extend battery life significantly.

## Physical Installation

### Mounting the Sensor

The accelerometer needs good contact with the machine to detect vibrations accurately. I found these work well:

1. **Magnetic mount** - if machine has steel casing, use neodymium magnets
2. **Velcro strips** - strong industrial velcro works surprisingly well
3. **Double-sided tape** - make sure it's the foam kind for vibration dampening

### Positioning

- Mount on the side or top of the machine
- Avoid mounting near hinges or doors (too much noise)
- Try to mount in same orientation on all machines for consistency

### Calibration

After mounting, you'll need to tune the thresholds in `config.h`:
- Run a washer/dryer through a full cycle
- Log the RMS values at each stage
- Adjust `IDLE_THRESHOLD`, `WASHING_MIN`, `WASHING_MAX`, `SPINNING_MIN` accordingly

My values work for the Maytag washers in my dorm but YMMV.

## Zigbee Network Setup

### Coordinator Setup

1. Flash one CC2652 with coordinator firmware
2. Connect to computer via USB
3. Should appear as `/dev/ttyUSB0` (Linux) or `/dev/tty.usbserial-*` (Mac)

### Adding End Devices

1. Flash end device firmware with unique `NODE_ID` in config.h
2. Power on the coordinator first
3. Power on end devices - they'll auto-join the network
4. Check coordinator logs to confirm devices joined

### Network Topology

The system uses a tree-based mesh:
```
        [Coordinator]
           /    \
        [N1]   [N2]
               /  \
            [N3]  [N4]
```

Each node can route messages, so you don't need line-of-sight to coordinator. Tested with up to 10 nodes but should handle 60+ according to Zigbee spec.

## Troubleshooting

### Accelerometer not detected
- Check I2C connections (SDA/SCL)
- Verify 3.3V power (NOT 5V!)
- Try a different I2C address (some boards use 0x1D)

### Node won't join network
- Make sure coordinator is powered on first
- Check Zigbee channel isn't congested (try channel 20 instead of 15)
- Verify `ZIGBEE_PAN_ID` matches between coordinator and nodes

### False readings
- Tighten mounting - loose sensor = noisy data
- Adjust thresholds in config.h
- Make sure machine is level (tilted machines vibrate differently)

### Poor range
- Zigbee should work up to 50m indoors
- Metal walls/floors block signal significantly
- Add router nodes to extend range

## Safety Notes

- Don't open the washing machines to install sensors inside (obvious but yeah)
- Make sure enclosures are water-resistant if near machines that might leak
- Secure cables so they don't get caught in doors/lids
- Check with facilities before deploying in dorms (learned this the hard way)
