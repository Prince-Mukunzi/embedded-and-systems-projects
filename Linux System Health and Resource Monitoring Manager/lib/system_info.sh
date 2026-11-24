#!/bin/bash

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
    local os_name

    os_name="$(uname -s)"
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
