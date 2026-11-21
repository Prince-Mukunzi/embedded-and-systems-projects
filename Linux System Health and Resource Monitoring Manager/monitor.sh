#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LOG_FILE="$SCRIPT_DIR/activity.log"
PROTECTED_NAMES="init systemd kthreadd sshd dbus-daemon login"

source "$SCRIPT_DIR/lib/utils.sh"
source "$SCRIPT_DIR/lib/system_info.sh"
source "$SCRIPT_DIR/lib/resources.sh"
source "$SCRIPT_DIR/lib/processes.sh"
source "$SCRIPT_DIR/lib/report.sh"

# Menu option 8
view_activity_log() {
    echo "========== ACTIVITY LOG =========="
    if [ ! -s "$LOG_FILE" ]; then
        echo "The activity log is empty."
        return
    fi

    cat "$LOG_FILE"
}

# Menu option 9
clear_activity_log() {
    if ! confirm "Are you sure you want to clear the activity log?"; then
        echo "The activity log was not cleared."
        return
    fi

    echo -n "" > "$LOG_FILE"
    echo "The activity log has been cleared."
    log_activity "Activity log cleared"
}

# Prints the main menu
show_menu() {
    echo ""
    echo "========================================"
    echo "   SYSTEM HEALTH AND RESOURCE MONITOR"
    echo "========================================"
    echo "1. Display system information"
    echo "2. Monitor CPU utilization"
    echo "3. Monitor memory utilization"
    echo "4. Monitor disk utilization"
    echo "5. View and search running processes"
    echo "6. Terminate a process"
    echo "7. Generate a system health report"
    echo "8. View activity log"
    echo "9. Clear activity log"
    echo "0. Exit"
    echo "========================================"
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
        2)
            monitor_cpu
            ;;
        3)
            monitor_memory
            ;;
        4)
            monitor_disk
            ;;
        5)
            view_processes
            ;;
        6)
            terminate_process
            ;;
        7)
            generate_report
            ;;
        8)
            view_activity_log
            ;;
        9)
            clear_activity_log
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
