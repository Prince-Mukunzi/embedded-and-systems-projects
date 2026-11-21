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

# Prints an error and returns failure when a needed command is not installed
command_exists() {
    if ! command -v "$1" > /dev/null 2>&1; then
        echo "Error: the '$1' command is not available on this system."
        log_activity "ERROR: missing command $1"
        return 1
    fi
    return 0
}

# Prints how long the system has been running
get_uptime() {
    if command -v uptime > /dev/null 2>&1; then
        uptime -p
    else
        echo "unknown (uptime command not available)"
    fi
}

# Prints hostname, operating system, kernel, uptime and the current date
print_system_info() {
    local os_name="$(uname -s)"

    if [ -f /etc/os-release ]; then
        os_name="$(grep '^PRETTY_NAME=' /etc/os-release | cut -d '"' -f 2)"
    fi

    echo "========== SYSTEM INFORMATION =========="
    echo "Hostname      : $HOSTNAME"
    echo "OS            : $os_name"
    echo "Kernel        : $(uname -r)"
    echo "Uptime        : $(get_uptime)"
    echo "Date and time : $(date)"
}

# Menu option 1
show_system_info() {
    print_system_info
    log_activity "Viewed system information"
}

# Prints the main menu
show_menu() {
    echo ""
    echo "========================================"
    echo "   SYSTEM HEALTH AND RESOURCE MONITOR"
    echo "========================================"
    echo "1. Display system information"
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
        1)
            show_system_info
            ;;
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
