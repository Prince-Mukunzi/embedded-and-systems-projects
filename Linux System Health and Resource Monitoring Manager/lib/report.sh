#!/bin/bash

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
