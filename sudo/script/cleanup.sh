#!/bin/bash
set -e
SERVICE_NAME="process.service"

SERVICE="process.service"
SERVICE_FILE="/etc/systemd/system/$SERVICE"

echo "Stopping and disabling service..."
systemctl stop "$SERVICE" 2>/dev/null || true
systemctl disable "$SERVICE" 2>/dev/null || true

echo "Removing files..."
rm -rf "/opt/process"
rm -f "/var/log/process.log"
rm -f "/run/process.pid"
rm -f "$SERVICE_FILE"

echo "Reloading systemd..."
systemctl daemon-reload

echo "Cleanup completed."