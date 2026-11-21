#!/bin/bash

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
