#include <Adafruit_LiquidCrystal.h>

#define ENTRY_BUTTON_PIN 6
#define EXIT_BUTTON_PIN 7
#define GREEN_LED_PIN 9
#define RED_LED_PIN 10
#define MAX_CAPACITY 5
#define DEBOUNCE_DELAY 50
#define LCD_UPDATE_INTERVAL 250
#define LCD_COLUMNS 16
#define LCD_ROWS 2
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

struct Button {
    int pin;
    int last_reading;
    int stable_state;
    unsigned long last_change_time;
};

// 0 selects the LCD's default I2C address (0x20), which is the Tinkercad default
Adafruit_LiquidCrystal lcd(0);
struct OccupancyInfo *occupancy = NULL;
unsigned long last_lcd_update = 0;
struct Button entry_button = {ENTRY_BUTTON_PIN, HIGH, HIGH, 0};
struct Button exit_button = {EXIT_BUTTON_PIN, HIGH, HIGH, 0};

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

// Returns true once for each real press. A reading only counts after it stays the same
// for DEBOUNCE_DELAY milliseconds, because a pressed button bounces between HIGH and LOW.
bool button_was_pressed(struct Button *button)
{
    int reading = digitalRead(button->pin);
    bool pressed = false;

    if (reading != button->last_reading) {
        button->last_change_time = millis();
        button->last_reading = reading;
    }

    if (millis() - button->last_change_time >= DEBOUNCE_DELAY && reading != button->stable_state) {
        button->stable_state = reading;
        // With INPUT_PULLUP the pin reads LOW while the button is held down
        if (button->stable_state == LOW) {
            pressed = true;
        }
    }
    return pressed;
}

// Adds one person if there is room, otherwise rejects the entry
void record_entry(struct OccupancyInfo *info)
{
    char message[MESSAGE_LENGTH];

    if (info->current_count >= info->max_capacity) {
        info->rejected_entries++;
        snprintf(message, MESSAGE_LENGTH, "ERROR: entry rejected, the facility is full (%d / %d).",
                 info->current_count, info->max_capacity);
        Serial.println(message);
        return;
    }

    info->current_count++;
    info->total_entries++;
    if (info->current_count > info->peak_count) {
        info->peak_count = info->current_count;
    }

    snprintf(message, MESSAGE_LENGTH, "ENTRY: %d inside, %d place(s) left.", info->current_count,
             remaining_capacity(info));
    Serial.println(message);
    if (info->current_count == info->max_capacity) {
        Serial.println("WARNING: maximum capacity reached. New entries will be rejected.");
    }
}

// Removes one person if anyone is inside, otherwise rejects the exit
void record_exit(struct OccupancyInfo *info)
{
    char message[MESSAGE_LENGTH];

    if (info->current_count <= 0) {
        info->rejected_exits++;
        Serial.println("ERROR: exit rejected, nobody is inside.");
        return;
    }

    info->current_count--;
    info->total_exits++;
    snprintf(message, MESSAGE_LENGTH, "EXIT: %d inside, %d place(s) left.", info->current_count,
             remaining_capacity(info));
    Serial.println(message);
}

// Green means there is room, red means the facility is full
void update_leds(const struct OccupancyInfo *info)
{
    if (info->current_count >= info->max_capacity) {
        digitalWrite(GREEN_LED_PIN, LOW);
        digitalWrite(RED_LED_PIN, HIGH);
    } else {
        digitalWrite(GREEN_LED_PIN, HIGH);
        digitalWrite(RED_LED_PIN, LOW);
    }
}

// Writes one full LCD row. The text is padded with spaces so old characters are overwritten,
// which avoids the flicker that lcd.clear() causes on every refresh.
void print_lcd_row(int row, const char *text)
{
    char padded[LCD_COLUMNS + 1];

    snprintf(padded, LCD_COLUMNS + 1, "%-16s", text);
    lcd.setCursor(0, row);
    lcd.print(padded);
}

// Shows the number of people inside and the places left
void update_lcd(const struct OccupancyInfo *info)
{
    char line[LCD_COLUMNS + 1];

    snprintf(line, LCD_COLUMNS + 1, "Inside: %d/%d", info->current_count, info->max_capacity);
    print_lcd_row(0, line);

    if (remaining_capacity(info) == 0) {
        print_lcd_row(1, "FULL - No entry");
    } else {
        snprintf(line, LCD_COLUMNS + 1, "Available: %d", remaining_capacity(info));
        print_lcd_row(1, line);
    }
}

// Replaces the record with a fresh one. The new record is made first, so if memory runs out
// the old record is kept and nothing is lost.
void reset_occupancy(struct OccupancyInfo **info_pointer)
{
    struct OccupancyInfo *fresh_record = create_occupancy_record(MAX_CAPACITY);

    if (fresh_record == NULL) {
        Serial.println("ERROR: not enough memory to reset. The current counts are kept.");
        return;
    }

    free(*info_pointer);
    *info_pointer = fresh_record;
    Serial.println("System reset: all counts are back to zero.");
}

// Reacts to a command typed in the Serial Monitor: s = statistics, r = reset
void handle_serial_command(struct OccupancyInfo **info_pointer)
{
    char command = (char) Serial.read();
    char message[MESSAGE_LENGTH];

    // The Serial Monitor sends Enter as '\n' or '\r', which are not commands
    if (command == '\n' || command == '\r') {
        return;
    }

    if (command == 's' || command == 'S') {
        print_statistics(*info_pointer);
    } else if (command == 'r' || command == 'R') {
        reset_occupancy(info_pointer);
        update_leds(*info_pointer);
    } else {
        snprintf(message, MESSAGE_LENGTH, "ERROR: unknown command '%c'. Use s or r.", command);
        Serial.println(message);
    }
}

void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);
    pinMode(ENTRY_BUTTON_PIN, INPUT_PULLUP);
    pinMode(EXIT_BUTTON_PIN, INPUT_PULLUP);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    lcd.begin(LCD_COLUMNS, LCD_ROWS);
    lcd.setBacklight(1);

    occupancy = create_occupancy_record(MAX_CAPACITY);
    if (occupancy == NULL) {
        Serial.println("ERROR: not enough memory for the occupancy record. System stopped.");
        digitalWrite(RED_LED_PIN, HIGH);
        print_lcd_row(0, "Memory error");
        print_lcd_row(1, "System stopped");
        return;
    }

    Serial.println("Smart Occupancy Monitoring System started.");
    Serial.println("Serial commands: s = show statistics, r = reset counts");
    print_statistics(occupancy);
    update_leds(occupancy);
    update_lcd(occupancy);
}

void loop()
{
    // Without a record there is nothing safe to do, so the system stays stopped
    if (occupancy == NULL) {
        return;
    }

    if (button_was_pressed(&entry_button)) {
        record_entry(occupancy);
        update_leds(occupancy);
    }
    if (button_was_pressed(&exit_button)) {
        record_exit(occupancy);
        update_leds(occupancy);
    }

    if (Serial.available() > 0) {
        handle_serial_command(&occupancy);
    }

    // millis() keeps the LCD refreshing without delay(), so button presses are never missed
    if (millis() - last_lcd_update >= LCD_UPDATE_INTERVAL) {
        last_lcd_update = millis();
        update_lcd(occupancy);
    }
}
