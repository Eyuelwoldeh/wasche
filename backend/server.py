"""
Wasche Backend Server
Receives data from Zigbee coordinator and stores in PostgreSQL

Flask API for querying machine status
"""

from flask import Flask, jsonify, request
from flask_cors import CORS
import psycopg2
from psycopg2.extras import RealDictCursor
import serial
import threading
import struct
import time
from datetime import datetime
import logging

app = Flask(__name__)
CORS(app)  # enable CORS for frontend

# Setup logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Database config
DB_CONFIG = {
    'host': 'localhost',
    'database': 'wasche_db',
    'user': 'wasche_user',
    'password': 'wasche_pass'  # yeah I know, should use env vars
}

# Serial port for Zigbee coordinator
SERIAL_PORT = '/dev/ttyUSB0'  # adjust for your system
SERIAL_BAUD = 115200

# Machine state mapping (matches firmware enum)
STATE_MAP = {
    0: 'IDLE',
    1: 'WASHING',
    2: 'SPINNING',
    3: 'DONE',
    4: 'UNKNOWN'
}

class ZigbeeReader(threading.Thread):
    """Thread that reads from Zigbee coordinator serial port"""
    
    def __init__(self, port, baudrate):
        super().__init__()
        self.port = port
        self.baudrate = baudrate
        self.running = False
        self.serial_conn = None
        
    def run(self):
        """Main loop - reads packets and processes them"""
        self.running = True
        
        try:
            self.serial_conn = serial.Serial(self.port, self.baudrate, timeout=1)
            logger.info(f"Connected to Zigbee coordinator on {self.port}")
        except Exception as e:
            logger.error(f"Failed to open serial port: {e}")
            return
        
        while self.running:
            try:
                # Read packet header to determine packet type
                packet_type = self.serial_conn.read(1)
                if not packet_type:
                    continue
                
                packet_type = struct.unpack('B', packet_type)[0]
                
                if packet_type == 0x01:  # DATA packet
                    self.process_data_packet()
                elif packet_type == 0x02:  # HEARTBEAT packet
                    self.process_heartbeat_packet()
                else:
                    logger.warning(f"Unknown packet type: {packet_type}")
                    
            except Exception as e:
                logger.error(f"Error reading from serial: {e}")
                time.sleep(1)
        
        if self.serial_conn:
            self.serial_conn.close()
    
    def process_data_packet(self):
        """Process a data packet from a node"""
        # Packet structure: type(1) + node_id(2) + state(1) + rms(4) + freq(4) + timestamp(4) + checksum(2)
        # Already read type, need 17 more bytes
        data = self.serial_conn.read(17)
        if len(data) != 17:
            logger.warning("Incomplete data packet")
            return
        
        # Unpack data
        node_id, state, rms, freq, timestamp, checksum = struct.unpack('<HBFFIH', data)
        
        # TODO: verify checksum
        
        state_str = STATE_MAP.get(state, 'UNKNOWN')
        logger.info(f"Node {node_id}: state={state_str}, rms={rms:.2f}, freq={freq:.1f}Hz")
        
        # Store in database
        try:
            conn = psycopg2.connect(**DB_CONFIG)
            cur = conn.cursor()
            
            cur.execute("""
                INSERT INTO machine_readings (node_id, machine_state, rms_magnitude, dominant_freq, timestamp)
                VALUES (%s, %s, %s, %s, %s)
            """, (node_id, state_str, rms, freq, datetime.now()))
            
            # Update machine status
            cur.execute("""
                INSERT INTO machine_status (node_id, current_state, last_updated)
                VALUES (%s, %s, %s)
                ON CONFLICT (node_id) 
                DO UPDATE SET current_state = %s, last_updated = %s
            """, (node_id, state_str, datetime.now(), state_str, datetime.now()))
            
            conn.commit()
            cur.close()
            conn.close()
            
        except Exception as e:
            logger.error(f"Database error: {e}")
    
    def process_heartbeat_packet(self):
        """Process a heartbeat packet"""
        data = self.serial_conn.read(17)  # same size as data packet
        if len(data) != 17:
            return
        
        node_id = struct.unpack('<H', data[:2])[0]
        logger.debug(f"Heartbeat from node {node_id}")
        
        # Update last_seen in database
        try:
            conn = psycopg2.connect(**DB_CONFIG)
            cur = conn.cursor()
            cur.execute("""
                UPDATE machine_status SET last_updated = %s WHERE node_id = %s
            """, (datetime.now(), node_id))
            conn.commit()
            cur.close()
            conn.close()
        except Exception as e:
            logger.error(f"Database error: {e}")
    
    def stop(self):
        """Stop the reader thread"""
        self.running = False

# API Routes

@app.route('/api/machines', methods=['GET'])
def get_machines():
    """Get all machines and their current status"""
    try:
        conn = psycopg2.connect(**DB_CONFIG)
        cur = conn.cursor(cursor_factory=RealDictCursor)
        
        cur.execute("""
            SELECT node_id, current_state, last_updated
            FROM machine_status
            ORDER BY node_id
        """)
        
        machines = cur.fetchall()
        cur.close()
        conn.close()
        
        return jsonify({
            'success': True,
            'machines': machines
        })
        
    except Exception as e:
        logger.error(f"Error fetching machines: {e}")
        return jsonify({
            'success': False,
            'error': str(e)
        }), 500

@app.route('/api/machines/<int:node_id>', methods=['GET'])
def get_machine_status(node_id):
    """Get detailed status for a specific machine"""
    try:
        conn = psycopg2.connect(**DB_CONFIG)
        cur = conn.cursor(cursor_factory=RealDictCursor)
        
        # Get current status
        cur.execute("""
            SELECT node_id, current_state, last_updated
            FROM machine_status
            WHERE node_id = %s
        """, (node_id,))
        
        status = cur.fetchone()
        
        if not status:
            return jsonify({
                'success': False,
                'error': 'Machine not found'
            }), 404
        
        # Get recent readings
        cur.execute("""
            SELECT machine_state, rms_magnitude, dominant_freq, timestamp
            FROM machine_readings
            WHERE node_id = %s
            ORDER BY timestamp DESC
            LIMIT 20
        """, (node_id,))
        
        readings = cur.fetchall()
        
        cur.close()
        conn.close()
        
        return jsonify({
            'success': True,
            'status': status,
            'recent_readings': readings
        })
        
    except Exception as e:
        logger.error(f"Error fetching machine status: {e}")
        return jsonify({
            'success': False,
            'error': str(e)
        }), 500

@app.route('/api/history/<int:node_id>', methods=['GET'])
def get_machine_history(node_id):
    """Get historical data for a machine"""
    # Get optional query params
    hours = request.args.get('hours', default=24, type=int)
    
    try:
        conn = psycopg2.connect(**DB_CONFIG)
        cur = conn.cursor(cursor_factory=RealDictCursor)
        
        cur.execute("""
            SELECT machine_state, rms_magnitude, dominant_freq, timestamp
            FROM machine_readings
            WHERE node_id = %s 
            AND timestamp > NOW() - INTERVAL '%s hours'
            ORDER BY timestamp ASC
        """, (node_id, hours))
        
        history = cur.fetchall()
        cur.close()
        conn.close()
        
        return jsonify({
            'success': True,
            'history': history
        })
        
    except Exception as e:
        logger.error(f"Error fetching history: {e}")
        return jsonify({
            'success': False,
            'error': str(e)
        }), 500

@app.route('/api/health', methods=['GET'])
def health_check():
    """Health check endpoint"""
    return jsonify({
        'success': True,
        'status': 'running',
        'timestamp': datetime.now().isoformat()
    })

if __name__ == '__main__':
    # Start Zigbee reader thread
    reader = ZigbeeReader(SERIAL_PORT, SERIAL_BAUD)
    reader.daemon = True
    reader.start()
    
    # Start Flask server
    logger.info("Starting Wasche backend server...")
    app.run(host='0.0.0.0', port=5000, debug=True)
