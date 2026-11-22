#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <errno.h>

#include "records.h"
#include "input.h"
#include "student.h"

struct CourseSummary {
    char code[CODE_LENGTH];
    char name[NAME_LENGTH];
    float total_percentage;
    int student_count;
};

struct ProgrammeSummary {
    char programme[NAME_LENGTH];
    int student_count;
};

// Returns the percentage a student scored in one course
float course_percentage(const struct Course *course)
{
    return course->marks / course->max_marks * 100.0f;
}

// Returns 1 if a percentage is a pass mark
int is_pass(float percentage)
{
    return percentage >= PASS_PERCENT;
}

// Linear search: returns the position of the course with this code, or -1 if not found
int find_course_index(const struct Student *student, const char *code)
{
    int i;

    for (i = 0; i < student->course_count; i++) {
        if (same_text_ignore_case(student->courses[i].code, code)) {
            return i;
        }
    }
    return -1;
}

// Menu option 6
void add_course(struct Student *students, int student_count)
{
    struct Course new_course;
    struct Course *resized;
    struct Student *student;
    int index = ask_for_student(students, student_count);

    if (index == -1) {
        return;
    }
    student = &students[index];

    if (!read_text("Course code: ", new_course.code, CODE_LENGTH)) {
        return;
    }
    if (find_course_index(student, new_course.code) != -1) {
        printf("Error: %s is already registered for this student.\n", new_course.code);
        return;
    }
    if (!read_text("Course name: ", new_course.name, NAME_LENGTH)) {
        return;
    }
    if (!read_float("Maximum marks: ", &new_course.max_marks) || !is_valid_max_marks(new_course.max_marks)) {
        printf("Error: maximum marks must be a number above 0 and up to %.0f.\n", MAX_MARKS_LIMIT);
        return;
    }
    if (!read_float("Marks obtained: ", &new_course.marks)
        || !is_valid_marks(new_course.marks, new_course.max_marks)) {
        printf("Error: marks must be a number from 0 to %.1f.\n", new_course.max_marks);
        return;
    }

    if (student->course_count == 0) {
        resized = calloc(1, sizeof(struct Course));
    } else {
        resized = realloc(student->courses, (student->course_count + 1) * sizeof(struct Course));
    }

    if (resized == NULL) {
        printf("Error: not enough memory to add the course.\n");
        return;
    }

    student->courses = resized;
    student->courses[student->course_count] = new_course;
    student->course_count++;
    printf("Course %s added for student %d.\n", new_course.code, student->id);
}

// Menu option 7
void update_course_marks(struct Student *students, int student_count)
{
    char code[INPUT_LENGTH];
    float new_marks;
    struct Course *course;
    int course_index;
    int index = ask_for_student(students, student_count);

    if (index == -1) {
        return;
    }
    if (students[index].course_count == 0) {
        printf("Error: this student has not registered any courses yet.\n");
        return;
    }

    display_student_details(&students[index]);
    read_line("\nEnter the course code to update: ", code, INPUT_LENGTH);

    course_index = find_course_index(&students[index], code);
    if (course_index == -1) {
        printf("Error: this student is not registered for course '%s'.\n", code);
        return;
    }
    course = &students[index].courses[course_index];

    printf("Current marks: %.1f out of %.1f\n", course->marks, course->max_marks);
    if (!read_float("New marks: ", &new_marks) || !is_valid_marks(new_marks, course->max_marks)) {
        printf("Error: marks must be a number from 0 to %.1f.\n", course->max_marks);
        return;
    }

    course->marks = new_marks;
    printf("Marks for %s updated to %.1f.\n", course->code, course->marks);
}

// Menu option 8
void display_performance(const struct Student *students, int student_count)
{
    const struct Student *student;
    const struct Course *course;
    float percentage;
    float average;
    int passed = 0;
    int i;
    int index = ask_for_student(students, student_count);

    if (index == -1) {
        return;
    }
    student = &students[index];

    printf("\n----- ACADEMIC PERFORMANCE: %s %s (%d) -----\n", student->first_name, student->last_name,
           student->id);
    if (student->course_count == 0) {
        printf("No courses registered, so there is no performance to show yet.\n");
        return;
    }

    printf("%-10s %-30s %8s %8s %8s %6s\n", "Code", "Course name", "Marks", "Max", "Percent", "Result");
    for (i = 0; i < student->course_count; i++) {
        course = &student->courses[i];
        percentage = course_percentage(course);
        if (is_pass(percentage)) {
            passed++;
        }
        printf("%-10s %-30s %8.1f %8.1f %7.1f%% %6s\n", course->code, course->name, course->marks,
               course->max_marks, percentage, is_pass(percentage) ? "PASS" : "FAIL");
    }

    average = calculate_average(student);
    printf("\nCourses passed  : %d of %d\n", passed, student->course_count);
    printf("Overall average : %.1f%%\n", average);
    printf("Overall result  : %s\n", is_pass(average) ? "PASS" : "FAIL");
}

// Prints the student with the ID the user types
void search_by_id(const struct Student *students, int student_count)
{
    int index = ask_for_student(students, student_count);

    if (index != -1) {
        print_table_header();
        print_student_row(&students[index]);
    }
}

// Linear search through every student. search_type 2 checks the names, 3 checks the programme.
void search_by_text(const struct Student *students, int student_count, int search_type)
{
    char search_text[INPUT_LENGTH];
    int found = 0;
    int is_match;
    int i;

    if (search_type == 2) {
        read_line("Enter the first or last name (or part of it): ", search_text, INPUT_LENGTH);
    } else {
        read_line("Enter the programme (or part of it): ", search_text, INPUT_LENGTH);
    }

    if (is_blank(search_text)) {
        printf("Error: the search text cannot be empty.\n");
        return;
    }

    for (i = 0; i < student_count; i++) {
        if (search_type == 2) {
            is_match = contains_ignore_case(students[i].first_name, search_text)
                       || contains_ignore_case(students[i].last_name, search_text);
        } else {
            is_match = contains_ignore_case(students[i].programme, search_text);
        }

        if (is_match) {
            if (found == 0) {
                print_table_header();
            }
            print_student_row(&students[i]);
            found++;
        }
    }

    if (found == 0) {
        printf("No students match '%s'.\n", search_text);
    } else {
        printf("%d student(s) found.\n", found);
    }
}

// Menu option 9
void search_students(const struct Student *students, int student_count)
{
    int option;

    if (student_count == 0) {
        printf("No students are registered yet.\n");
        return;
    }

    printf("\n----- SEARCH STUDENTS -----\n");
    printf("1. By student ID\n");
    printf("2. By first or last name\n");
    printf("3. By programme of study\n");
    if (!read_int("Choose an option: ", &option)) {
        option = 0;
    }

    switch (option) {
        case 1:
            search_by_id(students, student_count);
            break;
        case 2:
        case 3:
            search_by_text(students, student_count, option);
            break;
        default:
            printf("Error: please choose 1, 2 or 3.\n");
    }
}

// Returns 1 if student a must move after student b for the chosen sort order
int should_swap(const struct Student *a, const struct Student *b, int sort_type)
{
    int name_order;

    switch (sort_type) {
        case 1:
            return a->id > b->id;
        case 2:
            name_order = compare_ignore_case(a->first_name, b->first_name);
            if (name_order == 0) {
                name_order = compare_ignore_case(a->last_name, b->last_name);
            }
            return name_order > 0;
        case 3:
            return calculate_average(a) < calculate_average(b);
        default:
            return compare_ignore_case(a->programme, b->programme) > 0;
    }
}

// Bubble sort: swaps neighbours that are in the wrong order until a full pass makes no swaps
void bubble_sort_students(struct Student *students, int student_count, int sort_type)
{
    struct Student temp;
    int swapped;
    int i;
    int j;

    for (i = 0; i < student_count - 1; i++) {
        swapped = 0;

        for (j = 0; j < student_count - 1 - i; j++) {
            if (should_swap(&students[j], &students[j + 1], sort_type)) {
                temp = students[j];
                students[j] = students[j + 1];
                students[j + 1] = temp;
                swapped = 1;
            }
        }

        if (!swapped) {
            break;
        }
    }
}

// Menu option 10
void sort_students(struct Student *students, int student_count)
{
    int option;

    if (student_count == 0) {
        printf("No students are registered yet.\n");
        return;
    }

    printf("\n----- SORT STUDENTS -----\n");
    printf("1. By student ID\n");
    printf("2. By name (alphabetical)\n");
    printf("3. By overall average (highest first)\n");
    printf("4. By programme of study (alphabetical)\n");
    if (!read_int("Choose an option: ", &option) || option < 1 || option > 4) {
        printf("Error: please choose a number from 1 to 4.\n");
        return;
    }

    bubble_sort_students(students, student_count, option);
    printf("Students sorted.\n");
    display_all_students(students, student_count);
}

// Prints totals, overall average, best and worst student, and pass/fail counts
void print_overall_statistics(const struct Student *students, int student_count)
{
    const struct Student *highest = NULL;
    const struct Student *lowest = NULL;
    float average;
    float total_of_averages = 0.0f;
    int total_courses = 0;
    int assessed = 0;
    int passing = 0;
    int i;

    for (i = 0; i < student_count; i++) {
        total_courses += students[i].course_count;

        // Students without courses have no average, so they are left out of these numbers
        if (students[i].course_count == 0) {
            continue;
        }

        average = calculate_average(&students[i]);
        total_of_averages += average;
        assessed++;
        if (is_pass(average)) {
            passing++;
        }
        if (highest == NULL || average > calculate_average(highest)) {
            highest = &students[i];
        }
        if (lowest == NULL || average < calculate_average(lowest)) {
            lowest = &students[i];
        }
    }

    printf("Total registered students        : %d\n", student_count);
    printf("Total courses across all students: %d\n", total_courses);

    if (assessed == 0) {
        printf("No student has any courses yet, so there are no averages to report.\n");
        return;
    }

    printf("Overall average performance      : %.1f%%\n", total_of_averages / assessed);
    printf("Highest average                  : %s %s (%d) with %.1f%%\n", highest->first_name,
           highest->last_name, highest->id, calculate_average(highest));
    printf("Lowest average                   : %s %s (%d) with %.1f%%\n", lowest->first_name,
           lowest->last_name, lowest->id, calculate_average(lowest));
    printf("Students passing (average >= %.0f%%): %d\n", PASS_PERCENT, passing);
    printf("Students failing                 : %d\n", assessed - passing);
    printf("Students with no courses yet     : %d\n", student_count - assessed);
}

// Linear search: returns the position of the course code in the summary list, or -1
int find_course_summary(const struct CourseSummary *summaries, int summary_count, const char *code)
{
    int i;

    for (i = 0; i < summary_count; i++) {
        if (same_text_ignore_case(summaries[i].code, code)) {
            return i;
        }
    }
    return -1;
}

// Groups every course by its code and prints the average percentage for each one
void print_course_averages(const struct Student *students, int student_count)
{
    struct CourseSummary *summaries = NULL;
    struct CourseSummary *resized;
    const struct Course *course;
    int summary_count = 0;
    int position;
    int i;
    int j;

    for (i = 0; i < student_count; i++) {
        for (j = 0; j < students[i].course_count; j++) {
            course = &students[i].courses[j];
            position = find_course_summary(summaries, summary_count, course->code);

            if (position == -1) {
                resized = realloc(summaries, (summary_count + 1) * sizeof(struct CourseSummary));
                if (resized == NULL) {
                    printf("Error: not enough memory to build the course summary.\n");
                    free(summaries);
                    return;
                }
                summaries = resized;
                position = summary_count;
                strcpy(summaries[position].code, course->code);
                strcpy(summaries[position].name, course->name);
                summaries[position].total_percentage = 0.0f;
                summaries[position].student_count = 0;
                summary_count++;
            }

            summaries[position].total_percentage += course_percentage(course);
            summaries[position].student_count++;
        }
    }

    printf("\nAverage performance per course:\n");
    if (summary_count == 0) {
        printf("  No courses registered yet.\n");
    }
    for (i = 0; i < summary_count; i++) {
        printf("  %-10s %-30s %6.1f%%  (%d student(s))\n", summaries[i].code, summaries[i].name,
               summaries[i].total_percentage / summaries[i].student_count, summaries[i].student_count);
    }

    free(summaries);
}

// Counts how many students are in each programme of study and prints the counts
void print_programme_counts(const struct Student *students, int student_count)
{
    struct ProgrammeSummary *summaries = NULL;
    struct ProgrammeSummary *resized;
    int summary_count = 0;
    int position;
    int i;
    int j;

    for (i = 0; i < student_count; i++) {
        position = -1;
        for (j = 0; j < summary_count; j++) {
            if (same_text_ignore_case(summaries[j].programme, students[i].programme)) {
                position = j;
                break;
            }
        }

        if (position == -1) {
            resized = realloc(summaries, (summary_count + 1) * sizeof(struct ProgrammeSummary));
            if (resized == NULL) {
                printf("Error: not enough memory to build the programme summary.\n");
                free(summaries);
                return;
            }
            summaries = resized;
            position = summary_count;
            strcpy(summaries[position].programme, students[i].programme);
            summaries[position].student_count = 0;
            summary_count++;
        }

        summaries[position].student_count++;
    }

    printf("\nStudents per programme of study:\n");
    for (i = 0; i < summary_count; i++) {
        printf("  %-30s %d\n", summaries[i].programme, summaries[i].student_count);
    }

    free(summaries);
}

// Menu option 11
void generate_report(const struct Student *students, int student_count)
{
    if (student_count == 0) {
        printf("No students are registered yet, so there is nothing to report.\n");
        return;
    }

    printf("\n========== ACADEMIC PERFORMANCE REPORT ==========\n");
    print_overall_statistics(students, student_count);
    print_course_averages(students, student_count);
    print_programme_counts(students, student_count);
    printf("=================================================\n");
}

// Writes every student and their courses to a text file. Returns 1 on success.
int save_records(const struct Student *students, int student_count, const char *filename)
{
    FILE *file;
    const struct Student *student;
    const struct Course *course;
    int write_failed;
    int i;
    int j;

    file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: could not open %s for writing.\n", filename);
        return 0;
    }

    // Line 1 is the number of students, then each student line is followed by its course lines
    fprintf(file, "%d\n", student_count);
    for (i = 0; i < student_count; i++) {
        student = &students[i];
        fprintf(file, "%d|%s|%s|%s|%d|%d\n", student->id, student->first_name, student->last_name,
                student->programme, student->year, student->course_count);

        for (j = 0; j < student->course_count; j++) {
            course = &student->courses[j];
            fprintf(file, "%s|%s|%.2f|%.2f\n", course->code, course->name, course->marks, course->max_marks);
        }
    }

    // fclose also writes out anything still waiting in memory, so its result must be checked
    write_failed = ferror(file);
    if (fclose(file) != 0 || write_failed) {
        printf("Error: something went wrong while writing %s.\n", filename);
        return 0;
    }

    printf("%d student record(s) saved to %s.\n", student_count, filename);
    return 1;
}

// Reads the next line of the file and counts it. Returns 0 when the file has ended.
int read_file_line(FILE *file, char *line, int *line_number)
{
    if (fgets(line, FILE_LINE_LENGTH, file) == NULL) {
        return 0;
    }
    (*line_number)++;
    return 1;
}

// Fills a student from one saved line. Returns 1 only if every value is present and valid.
int parse_student_line(const char *line, struct Student *student)
{
    int fields = sscanf(line, "%d|%49[^|]|%49[^|]|%49[^|]|%d|%d", &student->id, student->first_name,
                        student->last_name, student->programme, &student->year, &student->course_count);

    return fields == 6 && student->id > 0 && is_valid_year(student->year) && student->course_count >= 0;
}

// Fills a course from one saved line. Returns 1 only if every value is present and valid.
int parse_course_line(const char *line, struct Course *course)
{
    int fields = sscanf(line, "%19[^|]|%49[^|]|%f|%f", course->code, course->name, &course->marks,
                        &course->max_marks);

    return fields == 4 && is_valid_max_marks(course->max_marks) && is_valid_marks(course->marks, course->max_marks);
}

// Reads one student line and the course lines under it. Returns 0 if anything is missing or invalid.
int load_one_student(FILE *file, struct Student *student, int *line_number)
{
    char line[FILE_LINE_LENGTH];
    int j;

    if (!read_file_line(file, line, line_number) || !parse_student_line(line, student)) {
        student->course_count = 0;
        return 0;
    }
    if (student->course_count == 0) {
        return 1;
    }

    student->courses = calloc(student->course_count, sizeof(struct Course));
    if (student->courses == NULL) {
        printf("Error: not enough memory to load the courses.\n");
        student->course_count = 0;
        return 0;
    }

    for (j = 0; j < student->course_count; j++) {
        if (!read_file_line(file, line, line_number) || !parse_course_line(line, &student->courses[j])) {
            return 0;
        }
    }
    return 1;
}

// Reads all records from the file into memory. Returns 0 if the file exists but cannot be loaded.
int load_records(struct Student **students, int *student_count, const char *filename)
{
    FILE *file;
    struct Student *loaded = NULL;
    char line[FILE_LINE_LENGTH];
    int line_number = 0;
    int total = 0;
    int ok = 1;
    int i;

    file = fopen(filename, "r");
    if (file == NULL) {
        if (errno == ENOENT) {
            printf("No saved records found. Starting with an empty list.\n");
            return 1;
        }
        printf("Error: %s exists but could not be opened.\n", filename);
        return 0;
    }

    if (!read_file_line(file, line, &line_number) || sscanf(line, "%d", &total) != 1 || total < 0) {
        ok = 0;
        total = 0;
    }

    // calloc sets every course pointer to NULL, so a half-loaded list can always be freed safely
    if (ok && total > 0) {
        loaded = calloc(total, sizeof(struct Student));
        if (loaded == NULL) {
            printf("Error: not enough memory to load the records.\n");
            ok = 0;
            total = 0;
        }
    }

    for (i = 0; ok && i < total; i++) {
        if (!load_one_student(file, &loaded[i], &line_number)) {
            ok = 0;
        } else if (find_student_index(loaded, i, loaded[i].id) != -1) {
            printf("Error: student ID %d appears twice in the file.\n", loaded[i].id);
            ok = 0;
        }
    }
    fclose(file);

    if (!ok) {
        printf("Error: %s could not be loaded (problem near line %d).\n", filename, line_number);
        free_all_students(loaded, total);
        return 0;
    }

    *students = loaded;
    *student_count = total;
    printf("%d student record(s) loaded from %s.\n", total, filename);
    return 1;
}

// Prints the main menu
void show_menu(void)
{
    printf("\n========================================\n");
    printf("  STUDENT ACADEMIC RECORDS SYSTEM\n");
    printf("========================================\n");
    printf(" 1. Register a new student\n");
    printf(" 2. Display all students\n");
    printf(" 3. View student details\n");
    printf(" 4. Update student information\n");
    printf(" 5. Delete a student\n");
    printf(" 6. Add a course to a student\n");
    printf(" 7. Update course marks\n");
    printf(" 8. Display a student's academic performance\n");
    printf(" 9. Search students\n");
    printf("10. Sort students\n");
    printf("11. Academic performance report\n");
    printf("12. Save records to file\n");
    printf(" 0. Save and exit\n");
    printf("========================================\n");
}

int main(void)
{
    struct Student *students = NULL;
    int student_count = 0;
    char input[INPUT_LENGTH];
    int choice;
    int running = 1;

    // Stopping here protects the file: saving an empty list on exit would overwrite it
    if (!load_records(&students, &student_count, DATA_FILE)) {
        printf("Fix or move %s, then start the program again.\n", DATA_FILE);
        return 1;
    }

    while (running) {
        show_menu();

        if (!read_line("Choose an option: ", input, INPUT_LENGTH)) {
            printf("\nInput ended. Saving before closing.\n");
            save_records(students, student_count, DATA_FILE);
            break;
        }

        if (!parse_int(input, &choice)) {
            printf("Error: please enter a number from the menu.\n");
            continue;
        }

        switch (choice) {
            case 1:
                register_student(&students, &student_count);
                break;
            case 2:
                display_all_students(students, student_count);
                break;
            case 3:
                view_student(students, student_count);
                break;
            case 4:
                update_student(students, student_count);
                break;
            case 5:
                delete_student(&students, &student_count);
                break;
            case 6:
                add_course(students, student_count);
                break;
            case 7:
                update_course_marks(students, student_count);
                break;
            case 8:
                display_performance(students, student_count);
                break;
            case 9:
                search_students(students, student_count);
                break;
            case 10:
                sort_students(students, student_count);
                break;
            case 11:
                generate_report(students, student_count);
                break;
            case 12:
                save_records(students, student_count, DATA_FILE);
                break;
            case 0:
                if (save_records(students, student_count, DATA_FILE)
                    || ask_yes_no("Saving failed. Exit without saving? (y/n): ")) {
                    running = 0;
                }
                break;
            default:
                printf("Error: %d is not a menu option.\n", choice);
        }
    }

    printf("Goodbye.\n");
    free_all_students(students, student_count);
    return 0;
}
