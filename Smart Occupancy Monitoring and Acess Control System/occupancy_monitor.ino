#include <LiquidCrystal.h>

#define MAX_CAPACITY 5
#define SERIAL_BAUD_RATE 9600
#define MESSAGE_LENGTH 64

struct OccupancyInfo {
    int current_count;
    int max_capacity;
    int total_entries;
    int total_exits;
    int rejected_entries;
    int rejected_exits;
    int peak_count;
};

struct OccupancyInfo *occupancy = NULL;

// Allocates a new occupancy record with every counter at zero. Returns NULL if memory ran out.
struct OccupancyInfo *create_occupancy_record(int max_capacity)
{
    struct OccupancyInfo *record = (struct OccupancyInfo *) calloc(1, sizeof(struct OccupancyInfo));

    if (record != NULL) {
        record->max_capacity = max_capacity;
    }
    return record;
}

// Returns how many more people are allowed in
int remaining_capacity(const struct OccupancyInfo *info)
{
    return info->max_capacity - info->current_count;
}

// Prints all counters to the Serial Monitor
void print_statistics(const struct OccupancyInfo *info)
{
    char message[MESSAGE_LENGTH];

    Serial.println("----- OCCUPANCY STATISTICS -----");
    snprintf(message, MESSAGE_LENGTH, "People inside     : %d / %d", info->current_count, info->max_capacity);
    Serial.println(message);
    snprintf(message, MESSAGE_LENGTH, "Places available  : %d", remaining_capacity(info));
    Serial.println(message);
    snprintf(message, MESSAGE_LENGTH, "Total entries     : %d", info->total_entries);
    Serial.println(message);
    snprintf(message, MESSAGE_LENGTH, "Total exits       : %d", info->total_exits);
    Serial.println(message);
    snprintf(message, MESSAGE_LENGTH, "Rejected entries  : %d", info->rejected_entries);
    Serial.println(message);
    snprintf(message, MESSAGE_LENGTH, "Rejected exits    : %d", info->rejected_exits);
    Serial.println(message);
    snprintf(message, MESSAGE_LENGTH, "Peak occupancy    : %d", info->peak_count);
    Serial.println(message);
    Serial.println("--------------------------------");
}

void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);

    occupancy = create_occupancy_record(MAX_CAPACITY);
    if (occupancy == NULL) {
        Serial.println("ERROR: not enough memory for the occupancy record. System stopped.");
        return;
    }

    Serial.println("Smart Occupancy Monitoring System started.");
    print_statistics(occupancy);
}

void loop()
{
    // Without a record there is nothing safe to do, so the system stays stopped
    if (occupancy == NULL) {
        return;
    }
}
