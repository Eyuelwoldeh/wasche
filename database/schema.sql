-- Wasche Database Schema
-- PostgreSQL database for storing laundry machine data

-- Drop existing tables if they exist
DROP TABLE IF EXISTS machine_readings CASCADE;
DROP TABLE IF EXISTS machine_status CASCADE;
DROP TABLE IF EXISTS nodes CASCADE;

-- Create nodes table (stores info about each Zigbee node)
CREATE TABLE nodes (
    node_id INTEGER PRIMARY KEY,
    location VARCHAR(100),
    machine_type VARCHAR(20) CHECK (machine_type IN ('washer', 'dryer')),
    building VARCHAR(50),
    floor INTEGER,
    installed_date TIMESTAMP DEFAULT NOW(),
    notes TEXT
);

-- Create machine_status table (current state of each machine)
CREATE TABLE machine_status (
    node_id INTEGER PRIMARY KEY REFERENCES nodes(node_id),
    current_state VARCHAR(20) CHECK (current_state IN ('IDLE', 'WASHING', 'SPINNING', 'DONE', 'UNKNOWN')),
    last_updated TIMESTAMP DEFAULT NOW(),
    is_online BOOLEAN DEFAULT TRUE
);

-- Create machine_readings table (historical data)
CREATE TABLE machine_readings (
    id SERIAL PRIMARY KEY,
    node_id INTEGER REFERENCES nodes(node_id),
    machine_state VARCHAR(20) CHECK (machine_state IN ('IDLE', 'WASHING', 'SPINNING', 'DONE', 'UNKNOWN')),
    rms_magnitude REAL,
    dominant_freq REAL,
    timestamp TIMESTAMP DEFAULT NOW()
);

-- Create indexes for faster queries
CREATE INDEX idx_readings_node_id ON machine_readings(node_id);
CREATE INDEX idx_readings_timestamp ON machine_readings(timestamp);
CREATE INDEX idx_readings_node_time ON machine_readings(node_id, timestamp);

-- Create function to check node online status
CREATE OR REPLACE FUNCTION check_node_online()
RETURNS TRIGGER AS $$
BEGIN
    -- Consider node offline if no update in 2 minutes
    IF NEW.last_updated < NOW() - INTERVAL '2 minutes' THEN
        NEW.is_online = FALSE;
    ELSE
        NEW.is_online = TRUE;
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

-- Create trigger for online status check
CREATE TRIGGER update_online_status
BEFORE UPDATE ON machine_status
FOR EACH ROW
EXECUTE FUNCTION check_node_online();

-- Insert some sample nodes (for testing)
-- In production these would be added as nodes are deployed
INSERT INTO nodes (node_id, location, machine_type, building, floor, notes) VALUES
(1, 'Laundry Room A - Left', 'washer', 'Mellby Hall', 1, 'Front-loading washer'),
(2, 'Laundry Room A - Center', 'washer', 'Mellby Hall', 1, 'Top-loading washer'),
(3, 'Laundry Room A - Right', 'dryer', 'Mellby Hall', 1, 'Standard dryer'),
(4, 'Laundry Room B - Left', 'washer', 'Ytterboe Hall', 2, 'Front-loading washer'),
(5, 'Laundry Room B - Right', 'dryer', 'Ytterboe Hall', 2, 'Standard dryer');

-- Initialize machine status for all nodes
INSERT INTO machine_status (node_id, current_state, is_online)
SELECT node_id, 'UNKNOWN', FALSE FROM nodes;

-- Create a view for easy querying of machine info
CREATE VIEW machine_info AS
SELECT 
    n.node_id,
    n.location,
    n.machine_type,
    n.building,
    n.floor,
    ms.current_state,
    ms.last_updated,
    ms.is_online
FROM nodes n
LEFT JOIN machine_status ms ON n.node_id = ms.node_id;

-- Grant permissions to wasche_user
-- (assuming user already exists, if not create it first)
GRANT ALL PRIVILEGES ON TABLE nodes TO wasche_user;
GRANT ALL PRIVILEGES ON TABLE machine_status TO wasche_user;
GRANT ALL PRIVILEGES ON TABLE machine_readings TO wasche_user;
GRANT ALL PRIVILEGES ON SEQUENCE machine_readings_id_seq TO wasche_user;
GRANT SELECT ON machine_info TO wasche_user;

-- Some useful queries for monitoring:

-- Get current status of all machines
-- SELECT * FROM machine_info ORDER BY building, floor, node_id;

-- Get machines that are currently available (IDLE or DONE)
-- SELECT * FROM machine_info WHERE current_state IN ('IDLE', 'DONE') AND is_online = TRUE;

-- Get usage statistics for the last 24 hours
-- SELECT 
--     node_id,
--     machine_state,
--     COUNT(*) as state_count,
--     AVG(rms_magnitude) as avg_vibration
-- FROM machine_readings
-- WHERE timestamp > NOW() - INTERVAL '24 hours'
-- GROUP BY node_id, machine_state
-- ORDER BY node_id;
