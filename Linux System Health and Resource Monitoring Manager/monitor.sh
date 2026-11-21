#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LOG_FILE="$SCRIPT_DIR/activity.log"
PROTECTED_NAMES="init systemd kthreadd sshd dbus-daemon login"

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

# Returns success only when the text is a whole positive number
is_valid_pid() {
    [[ "$1" =~ ^[0-9]+$ ]] && [ "$1" -gt 0 ]
}

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
    local name="$(ps -p "$pid" -o comm=)"
    local owner_uid="$(ps -p "$pid" -o uid= | tr -d ' ')"
    local protected

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
        5)
            view_processes
            ;;
        6)
            terminate_process
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
