#!/usr/bin/env python3
"""
Quick test script for the Wasche API
Run the server first, then run this
"""

import requests
import json

BASE_URL = "http://localhost:5000/api"

def test_health():
    """Test health check endpoint"""
    print("Testing health endpoint...")
    response = requests.get(f"{BASE_URL}/health")
    print(f"Status: {response.status_code}")
    print(f"Response: {json.dumps(response.json(), indent=2)}")
    print()

def test_get_machines():
    """Test getting all machines"""
    print("Testing get all machines...")
    response = requests.get(f"{BASE_URL}/machines")
    print(f"Status: {response.status_code}")
    data = response.json()
    if data['success']:
        print(f"Found {len(data['machines'])} machines")
        for machine in data['machines']:
            print(f"  Node {machine['node_id']}: {machine['current_state']}")
    print()

def test_get_machine_status(node_id=1):
    """Test getting status for specific machine"""
    print(f"Testing get machine {node_id} status...")
    response = requests.get(f"{BASE_URL}/machines/{node_id}")
    print(f"Status: {response.status_code}")
    data = response.json()
    if data['success']:
        print(f"Status: {data['status']}")
        print(f"Recent readings: {len(data['recent_readings'])} entries")
    print()

def test_get_history(node_id=1, hours=24):
    """Test getting historical data"""
    print(f"Testing get history for node {node_id}...")
    response = requests.get(f"{BASE_URL}/history/{node_id}?hours={hours}")
    print(f"Status: {response.status_code}")
    data = response.json()
    if data['success']:
        print(f"History: {len(data['history'])} readings in last {hours} hours")
    print()

if __name__ == "__main__":
    print("=" * 50)
    print("Wasche API Test Script")
    print("=" * 50)
    print()
    
    try:
        test_health()
        test_get_machines()
        test_get_machine_status(1)
        test_get_history(1, 24)
        
        print("All tests completed!")
        
    except requests.exceptions.ConnectionError:
        print("ERROR: Could not connect to server.")
        print("Make sure the backend server is running (python server.py)")
    except Exception as e:
        print(f"ERROR: {e}")
