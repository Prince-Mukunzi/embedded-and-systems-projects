#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LOG_FILE="$SCRIPT_DIR/activity.log"

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

# Prints the main menu
show_menu() {
    echo ""
    echo "========================================"
    echo "   SYSTEM HEALTH AND RESOURCE MONITOR"
    echo "========================================"
    echo "0. Exit"
    echo "========================================"
}

# Waits for Enter so the user can read the output before the menu comes back
pause() {
    echo ""
    read -r -p "Press Enter to return to the menu..."
}

check_environment
log_activity "Monitor started"

while true; do
    show_menu

    if ! read -r -p "Choose an option: " choice; then
        echo ""
        break
    fi

    case "$choice" in
        0)
            break
            ;;
        "")
            echo "Error: no option entered. Please type a number from the menu."
            ;;
        *)
            echo "Error: '$choice' is not a valid option. Please type a number from the menu."
            log_activity "Invalid menu option: $choice"
            ;;
    esac

    pause
done

echo "Exiting System Health Monitor. Goodbye."
log_activity "Monitor exited"
