# Smart Occupancy Monitoring and Access Control System

An Arduino Uno system, simulated in Tinkercad, that counts the people inside a facility with a maximum capacity of 5. One push button records a person entering and another records a person leaving. A 16x2 LCD shows how many people are inside and how many places are left, a green LED means there is room, and a red LED means the facility is full. Every event, warning and error is also printed to the Serial Monitor.

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
| LCD 16 x 2 (the plain one, not the I2C one) | 1 | Shows occupancy and places left |
| 220 ohm resistor | 3 | One for each LED and one for the LCD backlight |
| Breadboard | 1 | Circuit assembly |
| Jumper wires | As needed | Connections |

The buttons use the Arduino's built-in pull-up resistors (`INPUT_PULLUP`), so they need no resistor of their own.

## Wiring guide (Tinkercad)

Start with the Arduino and a breadboard. Connect the Arduino **5V** pin to the breadboard's red (+) rail and a **GND** pin to the blue (-) rail.

**Buttons**

Place each button across the gap in the middle of the breadboard. Use two legs that are diagonally opposite each other, because those are always on different sides of the switch.

| Button | One leg | Diagonally opposite leg |
|--------|---------|-------------------------|
| Entry | Arduino pin **6** | GND rail |
| Exit | Arduino pin **7** | GND rail |

**LEDs**

The longer bent leg in Tinkercad is the anode (+).

| LED | Anode (+) | Cathode (-) |
|-----|-----------|-------------|
| Green | 220 ohm resistor to Arduino pin **9** | GND rail |
| Red | 220 ohm resistor to Arduino pin **10** | GND rail |

**LCD 16 x 2**

Tinkercad shows the pin name when you hover over each pin.

| LCD pin | Connect to |
|---------|------------|
| GND | GND rail |
| VCC | 5V rail |
| V0 (contrast) | GND rail |
| RS | Arduino pin **12** |
| RW | GND rail |
| E | Arduino pin **11** |
| DB0, DB1, DB2, DB3 | Not connected |
| DB4 | Arduino pin **5** |
| DB5 | Arduino pin **4** |
| DB6 | Arduino pin **3** |
| DB7 | Arduino pin **2** |
| LED anode | 220 ohm resistor to the 5V rail |
| LED cathode | GND rail |

If the LCD text is hard to read, add a 10k potentiometer: its two outer pins go to 5V and GND, and its middle pin goes to V0 instead of GND.

**Code**

Open **Code**, switch from Blocks to **Text**, delete the example, paste all of `occupancy_monitor.ino`, then press **Start Simulation**. Open the **Serial Monitor** at the bottom of the code panel to see events, and type `s` or `r` there to send commands.

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

## Live demo checklist

1. Start the simulation: green LED on, LCD shows `Inside: 0/5` and `Available: 5`, statistics in the Serial Monitor.
2. Press Exit once: rejected (nobody inside).
3. Press Entry five times: the count goes up, then the red LED comes on and the LCD shows `FULL - No entry`.
4. Press Entry again: rejected with an error.
5. Press Exit: green LED back on.
6. Type `s` in the Serial Monitor for statistics, then `r` to reset.
