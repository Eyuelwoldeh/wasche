# Development Notes

Random thoughts and notes during development. Mostly for my own reference.

## October 2024

### Week 1
Started this project because I was tired of walking down to the laundry room only to find machines taken. Figured I could use accelerometers to detect when machines are running. Ordered some CC2652 boards and ADXL345 sensors from Adafruit.

### Week 2
Got the I2C communication working with the accelerometer. Had to read the datasheet like 5 times to understand the register addresses. Why do hardware datasheets have to be so dense?

The ADXL345 is pretty sensitive - picking up vibrations from me walking near it lol. Need to figure out signal processing.

### Week 3
Implemented a simple FFT to analyze frequency components. Turns out washers have pretty distinct vibration patterns:
- Idle: basically nothing (<0.1g)
- Washing cycle: moderate vibration, ~2-4Hz (the agitator)
- Spin cycle: intense vibration, 6-12Hz (spinning drum)

This should be enough to classify states!

### Week 4
Zigbee is kicking my ass. The TI Z-Stack documentation is confusing. Ended up simplifying and just implementing basic packet transmission. Not a full mesh yet but it works for now.

Got 3 nodes talking to a coordinator. Range is decent - works across my floor.

## November 2024

### Week 1
Backend server is up. Using Flask because it's simple. Postgres for the database - overkill maybe but wanted to practice SQL.

The serial port reading is finicky. Sometimes packets get corrupted. Need better error handling.

### Week 2
Deployed first real test in my dorm's laundry room. Taped sensors to machines with industrial velcro (thanks Amazon). Works pretty well!

Had to adjust thresholds - different machines vibrate differently. The old Maytag washers vibrate way more than the newer ones.

### Week 3
Facilities caught me installing sensors lol. Had to explain what I was doing. They were actually cool with it once I showed them it's non-invasive. Got permission to deploy in two laundry rooms.

Added a bunch of sample data to database for testing. Need to build a frontend eventually.

## December 2024

### Issues to Fix
- [ ] Power consumption is too high - nodes drain batteries in ~24hrs
  - Need to implement sleep mode
  - Maybe sample at lower rate when idle
- [ ] Sometimes nodes lose connection to coordinator
  - Need better reconnection logic
  - Add watchdog timer
- [ ] FFT is slow on the M4 core
  - Maybe pre-compute some lookup tables?
  - Or use a hardware accelerator?
- [ ] Checksum validation not implemented
  - Currently just computing but not verifying
  - Could lead to corrupted data in DB

### Ideas for Later
- Add push notifications when your laundry is done
- Web dashboard to see all machines
- Usage analytics (busiest times, etc.)
- Maybe add temp sensor to detect dryer heat?
- Mobile app would be sick

### Things I Learned
- Embedded systems are hard. Like really hard.
- I2C can be finicky - need pullup resistors
- Zigbee mesh networking is complex but powerful
- FFT math is cool but implementation is tricky
- PostgreSQL triggers are useful for auto-updating data
- Always test with real hardware, simulation only goes so far

### Resources I Used
- ADXL345 datasheet (Analog Devices)
- TI CC2652 Technical Reference Manual
- "Understanding Digital Signal Processing" by Richard Lyons (for FFT)
- Stack Overflow (obviously)
- TI E2E forums (surprisingly helpful)

## January 2025

System is running pretty stable now. Have 5 nodes deployed across two laundry rooms. Database has a few weeks of data.

Next step is building a simple web interface so people can actually use this. React maybe? Or just vanilla JS to keep it simple.

Also need to write this up for my resume. Pretty proud of this project tbh.

---

## Random Technical Notes

### Why 100Hz sampling rate?
Nyquist theorem says we need 2x the highest frequency we want to detect. Spin cycles go up to ~12Hz, so theoretically 24Hz would work. But I wanted some headroom and 100Hz gives nice clean FFT results with 128 samples.

### Buffer size = 128
Power of 2 for FFT efficiency. 128 samples at 100Hz = 1.28 seconds of data. Good balance between responsiveness and accuracy.

### Zigbee PAN ID = 0xFACE
lol yeah I know. Just thought it was funny. Should probably randomize this for security but whatever it's just laundry machines.

### Database indexes
Added indexes on (node_id, timestamp) for faster queries. Makes a huge difference when querying historical data.

### Why Flask not FastAPI?
Flask is just simpler and I know it better. This doesn't need to be super fast anyway. Might migrate later if it becomes a bottleneck.
