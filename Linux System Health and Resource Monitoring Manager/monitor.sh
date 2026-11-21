#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LOG_FILE="$SCRIPT_DIR/activity.log"
PROTECTED_NAMES="init systemd kthreadd sshd dbus-daemon login"

source "$SCRIPT_DIR/lib/utils.sh"
source "$SCRIPT_DIR/lib/system_info.sh"
source "$SCRIPT_DIR/lib/resources.sh"

# Lists the 20 processes using the most CPU
list_processes() {
    echo "Total running processes: $(ps -e --no-headers | wc -l)"
    echo "Top 20 processes by CPU usage:"
    ps -eo pid,user,comm,%cpu,%mem --sort=-%cpu | head -n 21
    log_activity "Listed running processes"
}

# Shows every process whose name contains the text the user typed
search_process_by_name() {
    local name
    local matches

    read -r -p "Enter the process name to search for: " name
    if [ -z "$name" ]; then
        echo "Error: the process name cannot be empty."
        return
    fi

    # index() does a plain text match, so symbols like * or . in the name are safe
    matches="$(ps -eo pid,user,comm,%cpu,%mem --no-headers | awk -v name="$name" \
        'index(tolower($3), tolower(name)) > 0')"

    if [ -z "$matches" ]; then
        echo "No running process matches '$name'."
        log_activity "Searched process name '$name': no match"
        return
    fi

    echo "  PID USER     COMMAND         %CPU %MEM"
    echo "$matches"
    log_activity "Searched process name '$name': $(echo "$matches" | wc -l) match(es)"
}

# Shows the process with the PID the user typed
search_process_by_pid() {
    local pid

    read -r -p "Enter the PID to search for: " pid
    if ! is_valid_pid "$pid"; then
        echo "Error: '$pid' is not a valid PID. A PID is a positive whole number."
        return
    fi

    if ! ps -p "$pid" > /dev/null; then
        echo "No running process has PID $pid."
        log_activity "Searched PID $pid: not found"
        return
    fi

    ps -p "$pid" -o pid,user,comm,%cpu,%mem
    log_activity "Searched PID $pid: found"
}

# Menu option 5
view_processes() {
    local option

    command_exists ps || return

    echo "========== PROCESSES =========="
    echo "1. List running processes"
    echo "2. Search by process name"
    echo "3. Search by PID"
    read -r -p "Choose an option: " option

    case "$option" in
        1) list_processes ;;
        2) search_process_by_name ;;
        3) search_process_by_pid ;;
        *) echo "Error: '$option' is not a valid option. Choose 1, 2 or 3." ;;
    esac
}

# Prints the reason a process must not be killed, or nothing if it is allowed
protection_reason() {
    local pid="$1"
    local name
    local owner_uid
    local protected

    name="$(ps -p "$pid" -o comm=)"
    owner_uid="$(ps -p "$pid" -o uid= | tr -d ' ')"

    if [ "$pid" -eq 1 ]; then
        echo "PID 1 is the init process that keeps the system running."
        return
    fi

    if [ "$pid" -eq "$$" ] || [ "$pid" -eq "$PPID" ]; then
        echo "it is this monitor script or the shell that started it."
        return
    fi

    for protected in $PROTECTED_NAMES; do
        if [ "$name" = "$protected" ]; then
            echo "'$name' is a protected system process."
            return
        fi
    done

    # Only root may stop processes that belong to other users
    if [ "$(id -u)" -ne 0 ] && [ "$owner_uid" != "$(id -u)" ]; then
        echo "you do not have permission to stop a process owned by another user."
    fi
}

# Menu option 6
terminate_process() {
    local pid
    local reason

    command_exists ps || return
    command_exists kill || return

    read -r -p "Enter the PID of the process to terminate: " pid
    if ! is_valid_pid "$pid"; then
        echo "Error: '$pid' is not a valid PID. A PID is a positive whole number."
        return
    fi

    if ! ps -p "$pid" > /dev/null; then
        echo "Error: no running process has PID $pid."
        log_activity "Terminate failed: PID $pid not found"
        return
    fi

    reason="$(protection_reason "$pid")"
    if [ -n "$reason" ]; then
        echo "Refused: cannot terminate PID $pid because $reason"
        log_activity "Terminate refused for PID $pid: $reason"
        return
    fi

    ps -p "$pid" -o pid,user,comm,%cpu,%mem
    if ! confirm "Are you sure you want to terminate this process?"; then
        echo "Termination cancelled."
        log_activity "Terminate cancelled for PID $pid"
        return
    fi

    if kill "$pid" 2> /dev/null; then
        echo "Process $pid has been sent the terminate signal."
        log_activity "Terminated process PID $pid"
    else
        echo "Error: could not terminate PID $pid (permission denied or it already ended)."
        log_activity "Terminate failed for PID $pid"
    fi
}

# Menu option 7
generate_report() {
    local report_dir
    local report_file

    command_exists top || return
    command_exists free || return
    command_exists df || return

    read -r -p "Enter the directory to save the report in: " report_dir
    if [ -z "$report_dir" ]; then
        echo "Error: the directory cannot be empty."
        return
    fi

    # read does not expand ~ like the shell does, so it is replaced by hand
    report_dir="${report_dir/#\~/$HOME}"

    if [ ! -d "$report_dir" ]; then
        echo "Error: the directory '$report_dir' does not exist."
        log_activity "Report failed: directory $report_dir does not exist"
        return
    fi

    if [ ! -w "$report_dir" ]; then
        echo "Error: you do not have permission to write in '$report_dir'."
        log_activity "Report failed: no write permission for $report_dir"
        return
    fi

    report_file="$report_dir/health_report_$(date +%Y%m%d_%H%M%S).txt"
    echo "Collecting system data, please wait..."

    {
        echo "SYSTEM HEALTH REPORT"
        echo "Generated on: $(timestamp)"
        echo ""
        print_system_info
        echo ""
        print_cpu_info "$(get_cpu_usage)"
        echo ""
        print_memory_info "$(get_memory_usage)"
        echo ""
        print_disk_info
    } > "$report_file"

    echo "Report saved to: $report_file"
    log_activity "Generated health report: $report_file"
}

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
