#!/bin/bash
# Wasche IoT System Setup Script
# Run this to set up the entire backend system

set -e

echo "╔════════════════════════════════════════╗"
echo "║   Wasche IoT Laundry System Setup     ║"
echo "╚════════════════════════════════════════╝"
echo ""

# Check if running on Linux/Mac
if [[ "$OSTYPE" != "linux-gnu"* ]] && [[ "$OSTYPE" != "darwin"* ]]; then
    echo "⚠️  This script is designed for Linux/Mac"
    echo "Windows users: manually follow the README instructions"
    exit 1
fi

# Check for required tools
echo "🔍 Checking dependencies..."

command -v python3 >/dev/null 2>&1 || { echo "❌ Python 3 is required but not installed."; exit 1; }
command -v psql >/dev/null 2>&1 || { echo "❌ PostgreSQL is required but not installed."; exit 1; }
command -v pip3 >/dev/null 2>&1 || { echo "❌ pip3 is required but not installed."; exit 1; }

echo "✅ All required tools found"
echo ""

# Setup Python virtual environment
echo "🐍 Setting up Python virtual environment..."
cd backend
python3 -m venv venv
source venv/bin/activate

echo "📦 Installing Python dependencies..."
pip install -r requirements.txt

cd ..

# Setup database
echo ""
echo "🗄️  Setting up database..."
read -p "Do you want to set up the PostgreSQL database? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    cd database
    chmod +x setup.sh
    ./setup.sh
    cd ..
    echo "✅ Database setup complete"
else
    echo "⏭️  Skipping database setup"
fi

# Make scripts executable
echo ""
echo "🔧 Making scripts executable..."
chmod +x backend/test_api.py
chmod +x database/setup.sh

echo ""
echo "╔════════════════════════════════════════╗"
echo "║        Setup Complete! 🎉              ║"
echo "╚════════════════════════════════════════╝"
echo ""
echo "Next steps:"
echo "1. Flash firmware to your CC2652 devices (see firmware/README.md)"
echo "2. Start the backend server:"
echo "   cd backend"
echo "   source venv/bin/activate"
echo "   python server.py"
echo "3. Test the API:"
echo "   python test_api.py"
echo ""
echo "For hardware setup, see docs/HARDWARE_SETUP.md"
echo ""
