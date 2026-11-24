#!/bin/bash

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
