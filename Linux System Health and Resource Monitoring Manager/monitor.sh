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

# Prints the CPU usage as a whole number percentage
get_cpu_usage() {
    local idle

    # The first sample from top is an average since boot, so the second one is used
    idle="$(top -bn2 -d 0.5 | grep 'Cpu(s)' | tail -n 1 | sed 's/.*, *\([0-9.]*\) id.*/\1/')"
    awk -v idle="$idle" 'BEGIN { printf "%.0f", 100 - idle }'
}

# Prints the given CPU usage and the processes using the most CPU
print_cpu_info() {
    echo "========== CPU UTILIZATION =========="
    echo "CPU usage: $1%"
    echo ""
    echo "Top 5 processes by CPU usage:"
    ps -eo pid,user,comm,%cpu,%mem --sort=-%cpu | head -n 6
}

# Menu option 2
monitor_cpu() {
    local threshold
    local cpu_usage

    command_exists top || return
    command_exists ps || return
    threshold="$(read_threshold "Enter the CPU warning threshold (1-100): ")" || return

    cpu_usage="$(get_cpu_usage)"
    print_cpu_info "$cpu_usage"
    echo ""

    if [ "$cpu_usage" -gt "$threshold" ]; then
        echo "WARNING: CPU usage is ${cpu_usage}%, above the ${threshold}% threshold."
        log_activity "WARNING: CPU usage ${cpu_usage}% exceeded threshold ${threshold}%"
    else
        echo "OK: CPU usage is ${cpu_usage}%, within the ${threshold}% threshold."
    fi
    log_activity "Checked CPU usage: ${cpu_usage}% (threshold ${threshold}%)"
}

# Prints the memory usage as a whole number percentage
get_memory_usage() {
    local total="$(free -m | awk '/^Mem:/ {print $2}')"
    local used="$(free -m | awk '/^Mem:/ {print $3}')"

    echo $(( used * 100 / total ))
}

# Prints total, used and available memory with the given usage percentage
print_memory_info() {
    echo "========== MEMORY UTILIZATION =========="
    echo "Total memory     : $(free -m | awk '/^Mem:/ {print $2}') MB"
    echo "Used memory      : $(free -m | awk '/^Mem:/ {print $3}') MB"
    echo "Available memory : $(free -m | awk '/^Mem:/ {print $7}') MB"
    echo "Memory usage     : $1%"
}

# Menu option 3
monitor_memory() {
    local threshold
    local memory_usage

    command_exists free || return
    threshold="$(read_threshold "Enter the memory warning threshold (1-100): ")" || return

    memory_usage="$(get_memory_usage)"
    print_memory_info "$memory_usage"
    echo ""

    if [ "$memory_usage" -gt "$threshold" ]; then
        echo "WARNING: memory usage is ${memory_usage}%, above the ${threshold}% threshold."
        log_activity "WARNING: memory usage ${memory_usage}% exceeded threshold ${threshold}%"
    else
        echo "OK: memory usage is ${memory_usage}%, within the ${threshold}% threshold."
    fi
    log_activity "Checked memory usage: ${memory_usage}% (threshold ${threshold}%)"
}

# Prints used and available space for each real mounted filesystem
print_disk_info() {
    echo "========== DISK UTILIZATION =========="
    df -hP -x tmpfs -x devtmpfs
}

# Menu option 4
monitor_disk() {
    local threshold
    local full_filesystems

    command_exists df || return
    threshold="$(read_threshold "Enter the disk warning threshold (1-100): ")" || return

    print_disk_info
    echo ""

    # Column 5 is "Use%" and column 6 is the mount point
    full_filesystems="$(df -P -x tmpfs -x devtmpfs | awk -v limit="$threshold" \
        'NR > 1 { usage = $5; sub("%", "", usage); if (usage + 0 > limit) print $6 " (" $5 ")" }')"

    if [ -n "$full_filesystems" ]; then
        echo "WARNING: these filesystems are above the ${threshold}% threshold:"
        echo "$full_filesystems"
        log_activity "WARNING: filesystems above disk threshold ${threshold}%: $(echo "$full_filesystems" | tr '\n' ' ')"
    else
        echo "OK: all filesystems are within the ${threshold}% threshold."
    fi
    log_activity "Checked disk usage (threshold ${threshold}%)"
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
        2)
            monitor_cpu
            ;;
        3)
            monitor_memory
            ;;
        4)
            monitor_disk
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
