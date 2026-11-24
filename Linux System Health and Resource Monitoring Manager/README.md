# Linux System Health and Resource Monitoring Manager

An interactive Bash program that checks the health of a Linux machine. From one menu you can see system information, check CPU, memory and disk usage against a warning threshold you choose, find and stop processes, save a timestamped health report, and review an activity log of everything you did.

## Requirements

- A Linux system (tested on Ubuntu 24.04). The script refuses to run on other systems.
- Standard Linux tools: `top`, `free`, `df`, `ps`, `kill`, `uptime`, `awk`, `sed`, `grep`. They come with every common distribution (package `procps`).

## How to run

```bash
cd "Linux System Health and Resource Monitoring Manager"
chmod +x monitor.sh
./monitor.sh
```

Run it with `sudo ./monitor.sh` if you want to be able to stop processes owned by other users.

## File structure

```
Linux System Health and Resource Monitoring Manager/
  monitor.sh               Entry point: settings, loads lib files, menu and main loop
  lib/
    utils.sh               Shared helpers: timestamp, logging, threshold input, y/n confirm, PID check
    system_info.sh         Hostname, OS, kernel, uptime, date
    resources.sh           CPU, memory and disk monitoring with thresholds
    processes.sh           List, search and safely terminate processes
    report.sh              Health report generation
    activity_log.sh        View and clear the activity log
  sample_health_report.txt Example report produced by option 7
  sample_activity.log      Example activity log from a full session
  activity.log             Created on first run (not committed)
```

`monitor.sh` finds the `lib` folder through `SCRIPT_DIR`, so the script works no matter which directory you run it from.

## Menu

```
1. Display system information
2. Monitor CPU utilization
3. Monitor memory utilization
4. Monitor disk utilization
5. View and search running processes
6. Terminate a process
7. Generate a system health report
8. View activity log
9. Clear activity log
0. Exit
```

## Functions

| File | Function | What it does |
|------|----------|--------------|
| utils.sh | `timestamp` | Returns the date and time as `YYYY-MM-DD HH:MM:SS` |
| utils.sh | `log_activity` | Adds one timestamped line to `activity.log` |
| utils.sh | `check_environment` | Exits if not on Linux or if the log cannot be written |
| utils.sh | `command_exists` | Prints an error if a needed command is missing |
| utils.sh | `read_threshold` | Keeps asking until the user enters a whole number 1-100 |
| utils.sh | `confirm` | Asks a y/n question, succeeds only on `y` or `Y` |
| utils.sh | `is_valid_pid` | Checks that the input is a positive whole number |
| utils.sh | `pause` | Waits for Enter before showing the menu again |
| system_info.sh | `get_uptime`, `print_system_info`, `show_system_info` | Option 1 |
| resources.sh | `get_cpu_usage`, `print_cpu_info`, `monitor_cpu` | Option 2 |
| resources.sh | `get_memory_usage`, `print_memory_info`, `monitor_memory` | Option 3 |
| resources.sh | `print_disk_info`, `monitor_disk` | Option 4 |
| processes.sh | `list_processes`, `search_process_by_name`, `search_process_by_pid`, `view_processes` | Option 5 |
| processes.sh | `protection_reason`, `terminate_process` | Option 6 |
| report.sh | `generate_report` | Option 7 |
| activity_log.sh | `view_activity_log`, `clear_activity_log` | Options 8 and 9 |

The `print_*` functions only print information. The menu functions call them and then check thresholds and write to the log. Because of this split, the report reuses the same `print_*` functions instead of repeating the code.

## Commands used

| Command | Used for |
|---------|----------|
| `uname -s`, `uname -r` | Check the system is Linux, show kernel version |
| `/etc/os-release` | Operating system name |
| `$HOSTNAME` | Hostname (built into Bash) |
| `uptime -p` | Uptime in a readable form |
| `date` | Current date and time, timestamps |
| `top -bn2 -d 0.5` | CPU idle percentage. The second sample is used because the first one is an average since boot |
| `ps -eo pid,user,comm,%cpu,%mem --sort=-%cpu` | Processes sorted by CPU usage |
| `ps -p PID` | Check if a PID exists and show its details |
| `free -m` | Total, used and available memory in MB |
| `df -hP -x tmpfs -x devtmpfs` | Disk usage of real filesystems (temporary memory filesystems are skipped) |
| `kill PID` | Sends the normal terminate signal (SIGTERM) |
| `awk`, `sed`, `grep`, `cut`, `tr`, `wc` | Pull single values out of command output |
| `command -v` | Check that a command is installed |

## Shell concepts used

- **Functions**: every menu option and every shared task is its own function (see table above).
- **Variables**: global settings in `monitor.sh` (`LOG_FILE`, `PROTECTED_NAMES`), `local` variables inside functions.
- **Conditionals**: `if` for every validation, `case` for the main menu and the process sub-menu.
- **Loops**: `while true` for the main menu and for `read_threshold`, `for` over `PROTECTED_NAMES`.
- **Command substitution**: `$(...)` everywhere a command's output is stored, for example `cpu_usage="$(get_cpu_usage)"`.
- **Regular expressions**: `[[ "$value" =~ ^[0-9]+$ ]]` checks that input is a whole number.

## Activity logging

Every important action is added to `activity.log` with a timestamp: start and exit, every check and its result, every warning, process searches, kills, refused kills, cancelled kills, reports created or failed, and invalid menu options. See `sample_activity.log`.

## Error handling and validation

| Situation | What the program does |
|-----------|-----------------------|
| Not running on Linux | Prints an error and exits |
| `lib` folder missing | Prints an error and exits |
| Cannot write `activity.log` | Prints an error and exits |
| Empty, letter or out-of-range menu choice | Prints an error, menu comes back |
| Threshold that is not a whole number 1-100 (e.g. `0`, `150`, `-5`, `1.5`, `abc`) | Asks again until it is valid |
| A needed command is missing (`top`, `free`, `df`, `ps`, `kill`) | Prints which command is missing, logs it, returns to the menu |
| Empty process name | Error message |
| No process matches the name | "No running process matches ..." |
| PID with letters, negative or zero | Error message |
| PID that is not running | "No running process has PID ..." |
| Trying to kill PID 1 | Refused, init keeps the system running |
| Trying to kill the monitor itself or its parent shell | Refused |
| Trying to kill a protected system process (`init`, `systemd`, `kthreadd`, `sshd`, `dbus-daemon`, `login`) | Refused |
| Normal user trying to kill another user's process | Refused, no permission |
| Kill confirmation answered with anything except `y` | Cancelled |
| `kill` itself fails | Error message |
| Report directory empty, missing, or not writable | Error message, nothing is written |
| Clear log answered with anything except `y` | Log is kept |
| Input ends (Ctrl+D) | The program exits cleanly |

## Testing

Every feature was tested in an Ubuntu 24.04 container: each menu option, every row of the table above, as both root and a normal user (to check permission refusals), with `top` and `free` removed (to check the missing-command errors). `shellcheck` reports no warnings.

## Sample output

```
========================================
   SYSTEM HEALTH AND RESOURCE MONITOR
========================================
...
Choose an option: 3
Enter the memory warning threshold (1-100): 20
========== MEMORY UTILIZATION ==========
Total memory     : 7836 MB
Used memory      : 2524 MB
Available memory : 5311 MB
Memory usage     : 32%

WARNING: memory usage is 32%, above the 20% threshold.
```

Full examples are in `sample_health_report.txt` and `sample_activity.log`.
