#!/bin/bash
# Setup script for Wasche database
# Creates database and user, then runs schema

set -e  # exit on error

echo "Setting up Wasche database..."

# Create database and user
sudo -u postgres psql << EOF
-- Create user if doesn't exist
DO \$\$
BEGIN
    IF NOT EXISTS (SELECT FROM pg_user WHERE usename = 'wasche_user') THEN
        CREATE USER wasche_user WITH PASSWORD 'wasche_pass';
    END IF;
END
\$\$;

-- Create database if doesn't exist
SELECT 'CREATE DATABASE wasche_db OWNER wasche_user'
WHERE NOT EXISTS (SELECT FROM pg_database WHERE datname = 'wasche_db')\gexec

-- Grant privileges
GRANT ALL PRIVILEGES ON DATABASE wasche_db TO wasche_user;

EOF

echo "Database and user created"

# Run schema
echo "Creating tables..."
psql -U wasche_user -d wasche_db -f schema.sql

echo "Database setup complete!"
echo ""
echo "Connection details:"
echo "  Host: localhost"
echo "  Database: wasche_db"
echo "  User: wasche_user"
echo "  Password: wasche_pass"
