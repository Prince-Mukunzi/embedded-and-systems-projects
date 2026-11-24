# Multi-threaded Document Search and Analysis System

A C program that searches several text files for one keyword at the same time, using POSIX threads. Each file gets its own thread, which counts the lines, counts every occurrence of the keyword (ignoring upper and lower case), records the line numbers where it appears, and writes a separate report file. The main program waits for every thread and then prints a summary.

## How to build and run

```bash
cd "Multi-threaded Document Search and Analysis System"
make
./document_analyzer "IoT" document1.txt document2.txt document3.txt
```

This creates `document1_report.txt`, `document2_report.txt` and `document3_report.txt` in the current folder. `make clean` removes the compiled program. Without `make`, the same build is:

```bash
gcc -Wall -Wextra -std=c11 -pthread -o document_analyzer main.c setup.c search.c report.c thread.c
```

The program needs a keyword and at least one file. Running it without them prints the usage:

```
Usage: ./document_analyzer "keyword" file1.txt [file2.txt ...]
```

## File structure

| File | What it contains |
|------|------------------|
| `analyzer.h` | All `#define` constants and `struct ThreadArgs` |
| `main.c` | Checks the arguments, creates one thread per file, joins them, prints the summary, frees memory |
| `setup.c` / `setup.h` | Usage message, keyword check, building report names, `initialize_thread_args`, report conflict check |
| `search.c` / `search.h` | Reading lines, lowercasing, counting keyword occurrences, recording line numbers, `analyze_file` |
| `report.c` / `report.h` | `generate_report` (one report file per input) and `display_results` (the terminal summary) |
| `thread.c` / `thread.h` | `process_file`, the function each thread runs |
| `Makefile` | Builds the program with `make` |
| `document1.txt` to `document3.txt` | Sample documents with "IoT" in mixed case, some lines with it twice |
| `sample_reports/` | The three reports and the terminal output from running the sample documents |

## The thread argument structure

```c
struct ThreadArgs {
    int thread_number;
    char filename[MAX_FILENAME];
    char keyword[MAX_KEYWORD];
    char report_name[REPORT_NAME_LENGTH];
    int total_lines;
    int occurrences;
    int matching_lines;
    int *line_numbers;
    int line_capacity;
    int status;
    int report_written;
    char error_message[ERROR_LENGTH];
};
```

- **Input to the thread:** `thread_number`, `filename`, `keyword` and `report_name` are filled in by `initialize_thread_args` before the thread starts.
- **Results from the thread:** `total_lines`, `occurrences`, `matching_lines`, `line_numbers`, `status` and `error_message` are filled in by the thread while it works.
- `line_numbers` is a dynamic array of the matching line numbers. `line_capacity` is how much room it has.
- `status` is `STATUS_PENDING`, `STATUS_COMPLETED` or `STATUS_FAILED`.

## How the threads work

1. `main` allocates three arrays with `calloc`, one place per file: `thread_args` (the structs), `threads` (the `pthread_t` ids) and `thread_created` (1 if that thread really started).
2. `initialize_thread_args` fills in each struct. `check_report_conflicts` then marks any file whose report would clash with another file's report (explained under error handling).
3. For every file that is still pending, `main` calls:

   ```c
   pthread_create(&threads[i], NULL, process_file, &thread_args[i])
   ```

   The fourth argument is a pointer to that file's own struct. That is how the filename and keyword reach the thread.
4. Each thread runs `process_file`, which turns the `void *` back into a `struct ThreadArgs *`, prints that it is processing the file, then calls `analyze_file` and `generate_report`, and prints success or the error.
5. `main` calls `pthread_join` on every thread with `thread_created[i] == 1`. Joining a thread that was never created is not allowed, which is why that array exists.
6. **How results come back:** the thread writes its results into the struct it was given. After `pthread_join` returns, the thread has finished, so `main` can safely read them. `display_results` prints the summary from those structs.

**Why no mutex is needed:** each thread only reads and writes its own `ThreadArgs`, its own input file and its own report file. Nothing is shared between threads, so they can never change the same data at the same time. Printing to the terminal is safe because each `printf` call prints its whole line at once. The brief says synchronization is not required, and a run with ThreadSanitizer (`-fsanitize=thread`) found no data races.

Because the threads really run at the same time, the order of the `[Thread N]` lines changes from run to run.

## Searching and counting

- `analyze_file` opens the file with `open_input_file`, then reads it one line at a time with `read_whole_line`.
- `read_whole_line` reads character by character with `fgetc` into a buffer that starts at 128 bytes and doubles with `realloc` whenever it is full, so lines of any length work.
- **Case-insensitive:** the keyword is copied and lowercased once, and every line is lowercased with `to_lowercase` (which uses `tolower`). So "IoT", "iot" and "IOT" all match.
- `count_occurrences` walks through the line with an index `i` and compares the keyword at each position with `strncmp`. On a match it adds 1 and jumps past the keyword, so a line with the keyword twice counts 2, and the same letters are never counted twice ("aa" in "aaaa" counts 2).
- If a line has at least one match, the count is added to `occurrences` and `record_matching_line` stores the line number.
- A keyword that is not found is not an error: the report is written with 0 occurrences and the status is Success.

## Dynamic memory

| Where | What happens |
|-------|--------------|
| `main` (main.c) | `calloc` the `thread_args`, `threads` and `thread_created` arrays, one place per file |
| `analyze_file` (search.c) | `malloc` a 128-byte line buffer, `free` it when the file is done |
| `read_whole_line` (search.c) | `realloc` the line buffer to double its size when a line is longer |
| `record_matching_line` (search.c) | `realloc` the `line_numbers` array: 8 places first, then double. `realloc` with a `NULL` pointer acts like `malloc`, so the first call needs no special case |
| end of `main` (main.c) | After all threads are joined: `free` every `line_numbers` array, then the three arrays |

`realloc` always goes into a temporary pointer (`resized`) first, so if it fails the old memory is still valid and gets freed normally. Every allocation is checked.

## Report file

Each thread writes `<file name without extension>_report.txt` into the current folder:

```
===== DOCUMENT ANALYSIS REPORT =====
Input file            : document1.txt
Search keyword        : IoT
Total lines processed : 9
Keyword occurrences   : 6
Matching lines        : 5
Line numbers          : 1, 2, 4, 5, 8
Status                : Success
```

A file that could not be processed still gets a report, with `Status : Failed (reason)`. `generate_report` checks `ferror` before `fclose` and checks the return value of `fclose`, so a failed write is reported instead of being treated as success.

## Error handling

| Situation | What happens |
|-----------|--------------|
| No keyword or no files | Error message and usage, exit code 1 |
| Empty or blank keyword (`""` or `"   "`) | Error message and usage, exit code 1 |
| Keyword longer than 99 characters | Error message and usage, exit code 1 |
| Input file does not exist | That thread fails with "the file does not exist"; the other threads continue |
| Input file has no read permission | That thread fails with "permission denied" |
| Input is a folder, or reading fails | That thread fails with "the file could not be read" |
| Empty file | Success with 0 lines |
| Keyword not found | Success with 0 occurrences, report still written |
| File name longer than 255 characters | Skipped before any thread starts |
| Same file given twice, or two files with the same name in different folders | The later one is skipped, because two threads writing the same report file would mix up the results |
| An input file is the report another file is about to write (for example `*.txt` in a folder with old reports) | That file is skipped, so a thread never reads a report while another thread writes it |
| Report file cannot be created or written | That file is marked Failed with the reason |
| `pthread_create` fails | That file is marked Failed, and it is never joined; the other threads continue |
| `malloc`, `calloc` or `realloc` returns `NULL` | Error message; nothing is leaked |

The exit code is 0 when every file completed and 1 when any file failed, so scripts can tell the difference.

## Testing

- Each function was tested right after it was written and committed only after it worked.
- Counts were checked against `grep -oi` and `grep -ni` on the sample documents, and on files with very long lines (100,000 characters), no final newline, blank lines, and more than 8 matches.
- On Linux (Ubuntu, gcc): zero warnings (also with `-O2`); AddressSanitizer with leak detection found no errors or leaks; ThreadSanitizer found no data races, including a run with 50 files. A run with 200 files completed every file correctly.
- `pthread_create` failure was tested by running the program as a normal user with a process limit (`ulimit -u`): every failed thread was reported and none was joined.
- Missing files, unreadable files, folders and unwritable output folders were tested as a normal user.
- After splitting `main.c` into modules, every test scenario produced exactly the same output and reports as the single-file version.

## Sample output

```
===== MULTI-THREADED DOCUMENT ANALYZER =====

Search Keyword: IoT
Number of Input Files: 3

[Thread 1] Processing document1.txt ...
[Thread 2] Processing document2.txt ...
[Thread 3] Processing document3.txt ...
[Thread 2] Successfully processed document2.txt
[Thread 1] Successfully processed document1.txt
[Thread 3] Successfully processed document3.txt

===== PROCESSING SUMMARY =====

document1.txt: Completed (6 occurrence(s) on 5 line(s), report: document1_report.txt)
document2.txt: Completed (5 occurrence(s) on 4 line(s), report: document2_report.txt)
document3.txt: Completed (4 occurrence(s) on 3 line(s), report: document3_report.txt)

All processing tasks completed.
Reports generated successfully.
```

The reports are in `sample_reports/`.
