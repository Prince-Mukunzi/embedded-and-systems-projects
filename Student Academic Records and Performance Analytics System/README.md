# Student Academic Records and Performance Analytics System

A menu-driven C program that keeps academic records for any number of students. You can register students, add their courses and marks, update or delete records, search and sort students, and print a performance report. Records live in dynamically allocated memory while the program runs and are saved to a text file so they are still there next time.

## How to build and run

```bash
cd "Student Academic Records and Performance Analytics System"
make
./records
```

To start with the sample data instead of an empty list:

```bash
cp sample_students.txt students.txt
./records
```

`make clean` removes the compiled program. Without `make`, the same build is:

```bash
gcc -Wall -Wextra -std=c11 -o records main.c input.c student.c course.c search_sort.c report.c file_io.c
```

## Menu

```
 1. Register a new student
 2. Display all students
 3. View student details
 4. Update student information
 5. Delete a student
 6. Add a course to a student
 7. Update course marks
 8. Display a student's academic performance
 9. Search students
10. Sort students
11. Academic performance report
12. Save records to file
 0. Save and exit
```

## File structure

| File | What it contains |
|------|------------------|
| `records.h` | All `#define` constants and the `struct Course` and `struct Student` definitions |
| `main.c` | The menu and the main loop that calls the right function for each option |
| `input.c` / `input.h` | Reading keyboard input safely and validating it (numbers, text, year, marks) |
| `student.c` / `student.h` | Register, display, view, update and delete students; calculate a student's average; free memory |
| `course.c` / `course.h` | Add courses, update marks, display a student's performance |
| `search_sort.c` / `search_sort.h` | Linear search by ID, name and programme; bubble sort by four orders |
| `report.c` / `report.h` | Academic performance report and the two small summary structs it uses |
| `file_io.c` / `file_io.h` | Saving records to `students.txt` and loading them back |
| `Makefile` | Builds the program with `make` |
| `sample_students.txt` | Six sample students with courses |
| `sample_report.txt` | The report produced from the sample data |

## Structures and how they relate

```c
struct Course {
    char code[CODE_LENGTH];
    char name[NAME_LENGTH];
    float marks;
    float max_marks;
};

struct Student {
    int id;
    char first_name[NAME_LENGTH];
    char last_name[NAME_LENGTH];
    char programme[NAME_LENGTH];
    int year;
    int course_count;
    struct Course *courses;
};
```

- The program keeps one dynamic array of students: `struct Student *students` plus `int student_count` (in `main`).
- Every student owns its own dynamic array of courses: `courses` points to `course_count` `struct Course` values.
- So it is a list of students, and each student has a list of courses.
- `report.h` adds two helper structs, `CourseSummary` and `ProgrammeSummary`, used only while the report is being built.

## Pointer usage

- Functions that only read the list take `const struct Student *students` (for example `display_all_students`, `search_students`, `generate_report`).
- Functions that change one student take `struct Student *` and use `->` (for example `student->course_count++` in `add_course`, `course->marks = new_marks` in `update_course_marks`).
- Functions that can move the whole list in memory take a pointer to the pointer: `register_student(struct Student **students, int *student_count)` and `delete_student(...)`. After `realloc` they write the new address back with `*students = resized` and change the count with `(*student_count)++`.
- `read_int`, `read_float` and the `parse_*` functions return their result through a pointer parameter and use the return value to say whether the input was valid.

## Dynamic memory allocation and deallocation

| Where | What happens |
|-------|--------------|
| `register_student` (student.c) | First student: `malloc`. Every next student: `realloc` to `count + 1` |
| `delete_student` (student.c) | `free` the student's courses, shift the rest left, `realloc` to `count - 1`. If the list becomes empty: `free` and set the pointer to `NULL` |
| `add_course` (course.c) | First course: `calloc`. Every next course: `realloc` to `course_count + 1` |
| `load_records`, `load_one_student` (file_io.c) | `calloc` the whole student list and each course list at the sizes stored in the file |
| `print_course_averages`, `print_programme_counts` (report.c) | Grow a summary list with `realloc`, `free` it at the end |
| `free_all_students` (student.c) | Frees every course list and then the student list. Called before the program exits |

Every allocation is checked. `realloc` always goes into a temporary pointer (`resized`) first, so if it returns `NULL` the old list is still valid and nothing is lost. The program prints "not enough memory" and returns to the menu.

## Searching and sorting algorithms

**Linear search** (`find_student_index`, `find_course_index`, `search_by_text`): look at every element from the first to the last and compare it.

- By ID: exact match.
- By name: the text can appear anywhere in the first or last name, ignoring case (`contains_ignore_case`).
- By programme: same idea, on the programme name.

**Bubble sort** (`bubble_sort_students`): go through the list comparing each pair of neighbours and swap them if they are in the wrong order. After each pass the largest item has "bubbled" to the end. If a pass makes no swaps, the list is already sorted and the loop stops early.

`should_swap` decides the order:

| Option | Order |
|--------|-------|
| 1 | Student ID, smallest first |
| 2 | First name, then last name, A to Z, ignoring case |
| 3 | Overall average, highest first |
| 4 | Programme, A to Z, ignoring case |

`qsort` is not used anywhere.

## File handling

Records are saved to `students.txt` as plain text, with `|` between fields:

```
6                                              <- number of students
1001|Alice|Mukamana|Software Engineering|2|3   <- id|first|last|programme|year|course_count
CS101|Programming in C|78.50|100.00            <- code|name|marks|max_marks  (3 of these)
...
```

- **Save** (`save_records`): option 12, and automatically when you choose 0. `|` is not allowed in any text field, so it can never break the file.
- **Load** (`load_records`): runs when the program starts. It reads the student count, `calloc`s the list, then reads each student line and its course lines with `sscanf`, so the dynamic arrays are rebuilt with exactly the right sizes.
- A missing file on the first run is normal: the program starts with an empty list.
- A damaged file (wrong format, missing lines, invalid year, marks above the maximum, a duplicate ID) stops the program with the line number. It does not start with an empty list, because saving that on exit would overwrite the real data.
- Marks are stored with 2 decimal places.

## Academic performance calculations

- **Course percentage** = `marks / max_marks * 100` (`course_percentage`).
- **Pass** = a percentage of at least 50% (`is_pass`, `PASS_PERCENT`).
- **Student overall average** = `total marks of all courses / total maximum marks of all courses * 100` (`calculate_average`). This weights each course by its maximum marks, as the brief asks.
- A student with no courses has no average. `calculate_average` checks `course_count == 0` before dividing, the table shows `N/A`, and the report leaves those students out of the averages and pass/fail counts.
- **Report** (`generate_report`): total students, total courses, overall average (the mean of the student averages), highest and lowest average student, passing and failing counts, students with no courses, average percentage per course code, and number of students per programme.

## Error handling and validation

| Situation | What happens |
|-----------|--------------|
| Menu choice that is empty, text, or not on the menu | Error message, menu shows again |
| Student ID that is not a positive whole number | Rejected |
| Student ID that already exists | Rejected ("already exists") |
| Empty or blank name, programme, course code or course name | Rejected |
| Text longer than the field allows | Rejected with the maximum length |
| Text containing `|` | Rejected (it is the file separator) |
| Year of study outside 1-6 | Rejected |
| Course code already registered for that student (any case) | Rejected |
| Maximum marks of 0 or less, above 1000, or not a number | Rejected |
| Marks below 0, above the maximum, or not a number (including `nan`, `inf`) | Rejected |
| View, update, delete or add a course for an ID that does not exist | "no student has ID ..." |
| Update marks for a course the student does not have | Error message |
| Delete without typing `y` | Cancelled |
| Update where one field is invalid | Nothing is changed (edits go to a copy first) |
| Searching or sorting with no students, search text empty | Error message |
| Report with no students, or students with no courses | Clear message, no division by zero |
| `malloc`, `calloc` or `realloc` returns `NULL` | Error message, existing data kept |
| Data file missing | Starts with an empty list |
| Data file exists but cannot be opened, or is damaged | Error with line number, program stops without touching the file |
| Data file cannot be written | Error message. On exit it asks whether to exit without saving |
| Keyboard input ends (Ctrl+D) | Saves and exits cleanly |

## Testing

- Every function was tested on its own right after it was written (happy path and every row of the table above), and committed only after it worked.
- A scripted session covering every menu option and the invalid cases was run with `-fsanitize=address,undefined` on Linux (gcc) and macOS (clang): no memory errors, no undefined behaviour, no memory leaks.
- Loading was tested with hand-made files: a bad count, a negative count, a missing student line, an invalid year, a missing course line, marks above the maximum, a duplicate ID, an unreadable file, an empty list and a valid file.
- After splitting `main.c` into modules, the same session produced exactly the same output and the same saved file as the single-file version.
- The code compiles with zero warnings using `-Wall -Wextra` on gcc (including `-O2`) and clang.

## Sample output

```
========== ACADEMIC PERFORMANCE REPORT ==========
Total registered students        : 6
Total courses across all students: 13
Overall average performance      : 59.9%
Highest average                  : Grace Uwase (1003) with 87.2%
Lowest average                   : Diane Ingabire (1005) with 38.4%
Students passing (average >= 50%): 3
Students failing                 : 2
Students with no courses yet     : 1
...
```

The full report is in `sample_report.txt`.
