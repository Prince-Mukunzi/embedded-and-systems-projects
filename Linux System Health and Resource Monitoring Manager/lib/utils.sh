#!/bin/bash

# Returns the current date and time in the format used by logs and reports
timestamp() {
    date "+%Y-%m-%d %H:%M:%S"
}

# Adds one timestamped line to the activity log
log_activity() {
    echo "[$(timestamp)] $1" >> "$LOG_FILE"
}

# Stops the script early if it is not running on Linux or cannot write its log
check_environment() {
    if [ "$(uname -s)" != "Linux" ]; then
        echo "Error: this script is designed for Linux only."
        exit 1
    fi

    if ! touch "$LOG_FILE" 2> /dev/null; then
        echo "Error: cannot write the activity log at $LOG_FILE"
        exit 1
    fi
}

# Prints an error and returns failure when a needed command is not installed
command_exists() {
    if ! command -v "$1" > /dev/null 2>&1; then
        echo "Error: the '$1' command is not available on this system."
        log_activity "ERROR: missing command $1"
        return 1
    fi
    return 0
}

# Keeps asking until the user types a whole number from 1 to 100, then prints it
read_threshold() {
    local value

    while true; do
        if ! read -r -p "$1" value; then
            return 1
        fi

        if [[ "$value" =~ ^[0-9]+$ ]] && [ "$value" -ge 1 ] && [ "$value" -le 100 ]; then
            echo "$value"
            return 0
        fi

        # Sent to stderr so it is shown to the user and not captured as the threshold
        echo "Invalid threshold. Enter a whole number from 1 to 100." >&2
    done
}

# Asks a yes/no question and returns success only when the user types y or Y
confirm() {
    local answer

    read -r -p "$1 (y/n): " answer
    [ "$answer" = "y" ] || [ "$answer" = "Y" ]
}

# Returns success only when the text is a whole positive number
is_valid_pid() {
    [[ "$1" =~ ^[0-9]+$ ]] && [ "$1" -gt 0 ]
}

# Waits for Enter so the user can read the output before the menu comes back
pause() {
    echo ""
    read -r -p "Press Enter to return to the menu..."
}
