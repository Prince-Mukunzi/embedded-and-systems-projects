# Smart Occupancy Monitoring and Access Control System

An Arduino Uno system, simulated in Tinkercad, that counts the people inside a facility with a maximum capacity of 5. One push button records a person entering and another records a person leaving. A 16x2 I2C LCD shows how many people are inside and how many places are left, a green LED means there is room, and a red LED means the facility is full. Every event, warning and error is also printed to the Serial Monitor.

## Files

| File | What it contains |
|------|------------------|
| `occupancy_monitor.ino` | The whole program. Tinkercad only accepts one code file, so everything is in this sketch, split into small functions |
| `README.md` | This document: wiring guide, how the code works, testing |

## Components

| Component | Quantity | Purpose |
|-----------|----------|---------|
| Arduino Uno R3 | 1 | Main controller |
| Pushbutton | 2 | Entry and exit |
| Green LED | 1 | Room available |
| Red LED | 1 | Facility full |
| LCD 16 x 2 (I2C) | 1 | Shows occupancy and places left |
| 220 ohm resistor | 2 | One for each LED |
| Breadboard Small | 1 | Holds the buttons, LEDs and resistors |
| Jumper wires | As needed | Connections |

The buttons use the Arduino's built-in pull-up resistors (`INPUT_PULLUP`), so they need no resistor of their own.

## Wiring guide (Tinkercad)

The LCD is the I2C version (Tinkercad's "LCD 16 x 2" with the I2C option, address 32 / 0x20). It needs only four wires instead of twelve, which keeps the circuit tidy. Arduino pins 2, 3, 4, 5, 11 and 12 are not used.

The breadboard sits to the right of the Arduino. Its columns are numbered 1 to 30 and its rows are lettered A to J. Rows A-E and F-J are separated by the gap in the middle. Each side has a red (+) and a blue (-) power rail.

**Ground**

| From | To | Wire colour |
|------|----|-------------|
| Arduino GND | Bottom (-) rail, column 1 | Black |
| Top (-) rail, column 30 | Bottom (-) rail, column 30 | Black |

The second wire joins both ground rails, so every part can reach GND from whichever rail is closest.

**Buttons**

Each button straddles the middle gap. Two diagonally opposite legs are always on different sides of the switch, so one goes to the Arduino pin and the other to GND.

| Button | Legs | Signal wire | Ground wire |
|--------|------|-------------|-------------|
| Entry | Columns 5 (row F) and 7 (row E) | Arduino pin **6** to column 5 row J (orange) | Column 7 row A to the bottom (-) rail (black) |
| Exit | Columns 11 (row F) and 13 (row E) | Arduino pin **7** to column 11 row J (blue) | Column 13 row A to the bottom (-) rail (black) |

**LEDs**

The bent leg in Tinkercad is the anode (+). Each LED sits in row J with its resistor in row H.

| LED | Cathode (-) | Anode (+) | Resistor | Signal wire |
|-----|-------------|-----------|----------|-------------|
| Green | Column 18, wired from row I to the top (-) rail | Column 19 | 220 ohm, column 19 to 23 | Arduino pin **9** to column 23 row J (green) |
| Red | Column 24, wired from row I to the top (-) rail | Column 25 | 220 ohm, column 25 to 29 | Arduino pin **10** to column 29 row J (purple) |

**LCD 16 x 2 (I2C)**

| LCD pin | Arduino pin | Wire colour |
|---------|-------------|-------------|
| GND | GND | Black |
| VCC | 5V | Red |
| SDA | SDA | Green |
| SCL | SCL | Yellow |

SDA and SCL are the two pins above AREF on the Arduino Uno.

**Code**

Open **Code**, switch from Blocks to **Text**, delete all of the starter code, paste all of `occupancy_monitor.ino`, then press **Start Simulation**. Open the **Serial Monitor** at the bottom of the code panel to see events, and type `s` or `r` there to send commands. The simulation runs slower while the code panel is open, so close it while pressing the buttons.

To press a button in the simulation, click and hold it for a moment. A very quick click can be shorter than the 50 ms debounce time and is then ignored, exactly as a real bounce would be.

## How it works

| Situation | Count | Green LED | Red LED | LCD row 2 | Serial Monitor |
|-----------|-------|-----------|---------|-----------|----------------|
| Start | 0 | ON | OFF | `Available: 5` | Startup message and statistics |
| Entry with room | +1 | ON | OFF | `Available: n` | `ENTRY: n inside, m place(s) left.` |
| Entry that fills the facility | 5 | OFF | ON | `FULL - No entry` | ENTRY line plus `WARNING: maximum capacity reached.` |
| Entry when full | stays 5 | OFF | ON | `FULL - No entry` | `ERROR: entry rejected, the facility is full (5 / 5).` |
| Exit | -1 | ON | OFF | `Available: n` | `EXIT: n inside, m place(s) left.` |
| Exit when empty | stays 0 | ON | OFF | `Available: 5` | `ERROR: exit rejected, nobody is inside.` |

Row 1 of the LCD always shows `Inside: n/5`.

**Serial commands** (type in the Serial Monitor and press Send):

| Command | What it does |
|---------|--------------|
| `s` | Prints the statistics: people inside, places left, total entries and exits, rejected entries and exits, peak occupancy |
| `r` | Resets every count to zero |
| anything else | `ERROR: unknown command` |

## Where each required concept is used

| Concept | Where in `occupancy_monitor.ino` |
|---------|----------------------------------|
| **Structures** | `struct OccupancyInfo` holds the count, capacity and statistics. `struct Button` holds one button's pin and debounce state |
| **Pointers** | Every function receives the record as `struct OccupancyInfo *info` and changes it through `info->current_count++` and similar (`record_entry`, `record_exit`). `button_was_pressed(struct Button *button)` updates the button's state through its pointer. `reset_occupancy(struct OccupancyInfo **info_pointer)` takes a pointer to the pointer so it can replace the record |
| **Dynamic memory** | `create_occupancy_record` allocates the record with `calloc`, which also sets every counter to 0 |
| **Memory management** | `setup` checks that `calloc` did not return `NULL`; if it did, the system stops safely with an error on the Serial Monitor and LCD and the red LED on. `reset_occupancy` creates the new record first, then `free`s the old one, so a failed allocation never loses the current data |
| **Functions** | One job each: `record_entry`, `record_exit`, `update_leds`, `update_lcd`, `print_lcd_row`, `print_statistics`, `remaining_capacity`, `button_was_pressed`, `handle_serial_command`, `reset_occupancy` |
| **Non-blocking programming** | `delay()` is not used anywhere. `button_was_pressed` uses `millis()` to accept a press only after the reading has been stable for 50 ms (debouncing). `loop` uses `millis()` to refresh the LCD every 250 ms |
| **Error handling** | Entry when full and exit when empty are rejected, counted and reported. Unknown serial commands are reported. Memory failures are handled |
| **Serial communication** | Every entry, exit, warning, rejection and statistics report goes to the Serial Monitor at 9600 baud |

**Why debouncing is needed:** when a real button is pressed, its metal contacts bounce and the pin flips between HIGH and LOW for a few milliseconds. Without debouncing, one press could be counted as several people. `button_was_pressed` remembers when the reading last changed and only accepts it once it has stayed the same for `DEBOUNCE_DELAY` (50 ms), and it reports each press only once, however long the button is held.

**Why `free` is only used in the reset:** an Arduino program never ends, because `loop()` runs forever, so the occupancy record is needed for the whole time the board is on. The one place the program throws a record away is the reset command, and that is where the old record is freed.

## Testing

The sketch's logic was tested on a computer before being put in Tinkercad. A small test program provided stand-ins for the Arduino functions (`digitalRead`, `digitalWrite`, `millis`, the LCD and the Serial Monitor), simulated time one millisecond at a time, and pressed the buttons with realistic contact bounce. It was built with `-Wall -Wextra` (no warnings) and AddressSanitizer (no memory errors).

| Test | Result |
|------|--------|
| Each bouncing press | Counted exactly once |
| 6 entries | Count stops at 5, sixth entry rejected with an error, red LED on, LCD shows FULL |
| Exits down to 0, then 2 more | Count stops at 0, extra exits rejected with an error |
| LED table from the brief (0, partly full, full, entry when full) | Every row matched |
| LCD | Updated within 250 ms after each event, no `delay()` called |
| `s`, `r`, unknown command | Statistics correct; reset sets everything to 0 and turns the green LED back on; unknown command reported |
| `calloc` fails at startup | System stops with an error on the Serial Monitor and LCD, red LED on, buttons ignored |
| `calloc` fails during reset | Error message, old counts kept |

The finished circuit was then run in Tinkercad: exit when empty was rejected, five entries filled the facility (red LED on, LCD `FULL - No entry`), the sixth entry was rejected, an exit turned the green LED back on, and `s`, `r` and an unknown command all gave the expected output.

## Live demo checklist

1. Start the simulation: green LED on, LCD shows `Inside: 0/5` and `Available: 5`, statistics in the Serial Monitor.
2. Press Exit once: rejected (nobody inside).
3. Press Entry five times: the count goes up, then the red LED comes on and the LCD shows `FULL - No entry`.
4. Press Entry again: rejected with an error.
5. Press Exit: green LED back on.
6. Type `s` in the Serial Monitor for statistics, then `r` to reset.
