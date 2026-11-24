#!/bin/bash

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
    local total
    local used

    total="$(free -m | awk '/^Mem:/ {print $2}')"
    used="$(free -m | awk '/^Mem:/ {print $3}')"
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
