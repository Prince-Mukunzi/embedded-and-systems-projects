#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>

#define DATA_FILE "students.txt"
#define NAME_LENGTH 50
#define CODE_LENGTH 20
#define INPUT_LENGTH 100
#define PASS_PERCENT 50.0f
#define MIN_YEAR 1
#define MAX_YEAR 6
#define MAX_MARKS_LIMIT 1000.0f

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

// Reads one line from the keyboard without the newline. Returns 0 when input has ended.
int read_line(const char *prompt, char *buffer, int size)
{
    int length;
    int c;

    printf("%s", prompt);
    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
        return 0;
    }

    length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    } else {
        // The line was longer than the buffer, so the rest of it is thrown away
        while ((c = getchar()) != '\n' && c != EOF) {
        }
    }
    return 1;
}

// Converts text to a whole number. Returns 1 only if the whole text is a valid number.
int parse_int(const char *text, int *value)
{
    char *end;
    long number;

    if (text[0] == '\0') {
        return 0;
    }

    number = strtol(text, &end, 10);
    if (*end != '\0' || number < INT_MIN || number > INT_MAX) {
        return 0;
    }

    *value = (int) number;
    return 1;
}

// Asks for a whole number. Returns 1 if the user typed a valid one.
int read_int(const char *prompt, int *value)
{
    char buffer[INPUT_LENGTH];

    read_line(prompt, buffer, INPUT_LENGTH);
    return parse_int(buffer, value);
}

// Returns 1 if the text has only spaces or tabs in it
int is_blank(const char *text)
{
    int i;

    for (i = 0; text[i] != '\0'; i++) {
        if (!isspace((unsigned char) text[i])) {
            return 0;
        }
    }
    return 1;
}

// Checks that text is not empty, fits in the field and has no '|' (the save file separator)
int is_valid_text(const char *text, int size)
{
    if (is_blank(text)) {
        printf("Error: this field cannot be empty.\n");
        return 0;
    }
    if ((int) strlen(text) >= size) {
        printf("Error: the text is too long (maximum %d characters).\n", size - 1);
        return 0;
    }
    if (strchr(text, '|') != NULL) {
        printf("Error: the '|' character is not allowed.\n");
        return 0;
    }
    return 1;
}

// Asks for a text field and copies it into destination only if it is valid
int read_text(const char *prompt, char *destination, int size)
{
    char buffer[INPUT_LENGTH];

    read_line(prompt, buffer, INPUT_LENGTH);
    if (!is_valid_text(buffer, size)) {
        return 0;
    }

    strcpy(destination, buffer);
    return 1;
}

// Returns 1 if the year of study is in the allowed range
int is_valid_year(int year)
{
    return year >= MIN_YEAR && year <= MAX_YEAR;
}

// Linear search: returns the position of the student with this ID, or -1 if not found
int find_student_index(const struct Student *students, int student_count, int id)
{
    int i;

    for (i = 0; i < student_count; i++) {
        if (students[i].id == id) {
            return i;
        }
    }
    return -1;
}

// Menu option 1: asks for the student's details and adds them to the list
void register_student(struct Student **students, int *student_count)
{
    struct Student new_student;
    struct Student *resized;

    if (!read_int("Student ID: ", &new_student.id) || new_student.id <= 0) {
        printf("Error: the student ID must be a positive whole number.\n");
        return;
    }
    if (find_student_index(*students, *student_count, new_student.id) != -1) {
        printf("Error: a student with ID %d already exists.\n", new_student.id);
        return;
    }

    if (!read_text("First name: ", new_student.first_name, NAME_LENGTH)) {
        return;
    }
    if (!read_text("Last name: ", new_student.last_name, NAME_LENGTH)) {
        return;
    }
    if (!read_text("Programme of study: ", new_student.programme, NAME_LENGTH)) {
        return;
    }
    if (!read_int("Year of study (1-6): ", &new_student.year) || !is_valid_year(new_student.year)) {
        printf("Error: the year of study must be a whole number from %d to %d.\n", MIN_YEAR, MAX_YEAR);
        return;
    }

    new_student.course_count = 0;
    new_student.courses = NULL;

    if (*student_count == 0) {
        resized = malloc(sizeof(struct Student));
    } else {
        resized = realloc(*students, (*student_count + 1) * sizeof(struct Student));
    }

    // On failure the old list is still valid, so nothing is lost
    if (resized == NULL) {
        printf("Error: not enough memory to register the student.\n");
        return;
    }

    *students = resized;
    (*students)[*student_count] = new_student;
    (*student_count)++;
    printf("Student %d registered successfully.\n", new_student.id);
}

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

// Overall average as a percentage: total marks divided by total maximum marks, times 100
float calculate_average(const struct Student *student)
{
    float total_marks = 0.0f;
    float total_max_marks = 0.0f;
    int i;

    // No courses means nothing to divide by, so the average is reported as 0
    if (student->course_count == 0) {
        return 0.0f;
    }

    for (i = 0; i < student->course_count; i++) {
        total_marks += student->courses[i].marks;
        total_max_marks += student->courses[i].max_marks;
    }
    return total_marks / total_max_marks * 100.0f;
}

// Prints the column titles used by every student table
void print_table_header(void)
{
    printf("%-8s %-30s %-25s %-5s %-7s %8s\n", "ID", "Name", "Programme", "Year", "Courses", "Average");
    printf("--------------------------------------------------------------------------------------\n");
}

// Prints one student as a row of the table
void print_student_row(const struct Student *student)
{
    char full_name[NAME_LENGTH * 2];

    snprintf(full_name, sizeof(full_name), "%s %s", student->first_name, student->last_name);
    printf("%-8d %-30s %-25s %-5d %-7d ", student->id, full_name, student->programme,
           student->year, student->course_count);

    if (student->course_count == 0) {
        printf("%8s\n", "N/A");
    } else {
        printf("%7.1f%%\n", calculate_average(student));
    }
}

// Menu option 2: prints every registered student
void display_all_students(const struct Student *students, int student_count)
{
    int i;

    if (student_count == 0) {
        printf("No students are registered yet.\n");
        return;
    }

    print_table_header();
    for (i = 0; i < student_count; i++) {
        print_student_row(&students[i]);
    }
    printf("Total: %d student(s)\n", student_count);
}

// Asks for a student ID and returns that student's position, or -1 with an error message
int ask_for_student(const struct Student *students, int student_count)
{
    int id;
    int index;

    if (student_count == 0) {
        printf("No students are registered yet.\n");
        return -1;
    }
    if (!read_int("Enter the student ID: ", &id)) {
        printf("Error: the student ID must be a whole number.\n");
        return -1;
    }

    index = find_student_index(students, student_count, id);
    if (index == -1) {
        printf("Error: no student has ID %d.\n", id);
    }
    return index;
}

// Prints all the personal details and courses of one student
void display_student_details(const struct Student *student)
{
    int i;

    printf("\n----- STUDENT DETAILS -----\n");
    printf("Student ID : %d\n", student->id);
    printf("Name       : %s %s\n", student->first_name, student->last_name);
    printf("Programme  : %s\n", student->programme);
    printf("Year       : %d\n", student->year);
    printf("Courses    : %d\n", student->course_count);

    if (student->course_count == 0) {
        printf("This student has not registered any courses yet.\n");
        return;
    }

    printf("\n%-10s %-30s %8s %8s\n", "Code", "Course name", "Marks", "Max");
    for (i = 0; i < student->course_count; i++) {
        printf("%-10s %-30s %8.1f %8.1f\n", student->courses[i].code, student->courses[i].name,
               student->courses[i].marks, student->courses[i].max_marks);
    }
}

// Menu option 3
void view_student(const struct Student *students, int student_count)
{
    int index = ask_for_student(students, student_count);

    if (index != -1) {
        display_student_details(&students[index]);
    }
}

// Asks for a new value for a text field. Pressing Enter keeps the current value.
int update_text_field(const char *label, char *field, int size)
{
    char prompt[128];
    char buffer[INPUT_LENGTH];

    snprintf(prompt, sizeof(prompt), "%s [%s]: ", label, field);
    read_line(prompt, buffer, INPUT_LENGTH);

    if (buffer[0] == '\0') {
        return 1;
    }
    if (!is_valid_text(buffer, size)) {
        return 0;
    }

    strcpy(field, buffer);
    return 1;
}

// Menu option 4
void update_student(struct Student *students, int student_count)
{
    struct Student updated;
    char prompt[64];
    char buffer[INPUT_LENGTH];
    int index = ask_for_student(students, student_count);

    if (index == -1) {
        return;
    }

    // Changes go into a copy first, so a mistake halfway through leaves the record unchanged
    updated = students[index];
    printf("Press Enter to keep the current value. The student ID cannot be changed.\n");

    if (!update_text_field("First name", updated.first_name, NAME_LENGTH)) {
        return;
    }
    if (!update_text_field("Last name", updated.last_name, NAME_LENGTH)) {
        return;
    }
    if (!update_text_field("Programme of study", updated.programme, NAME_LENGTH)) {
        return;
    }

    snprintf(prompt, sizeof(prompt), "Year of study (1-6) [%d]: ", updated.year);
    read_line(prompt, buffer, INPUT_LENGTH);
    if (buffer[0] != '\0') {
        if (!parse_int(buffer, &updated.year) || !is_valid_year(updated.year)) {
            printf("Error: the year of study must be a whole number from %d to %d.\n", MIN_YEAR, MAX_YEAR);
            return;
        }
    }

    students[index] = updated;
    printf("Student %d updated successfully.\n", updated.id);
}

// Asks a yes/no question. Returns 1 only if the user types y or Y.
int ask_yes_no(const char *question)
{
    char buffer[INPUT_LENGTH];

    read_line(question, buffer, INPUT_LENGTH);
    return strcmp(buffer, "y") == 0 || strcmp(buffer, "Y") == 0;
}

// Menu option 5
void delete_student(struct Student **students, int *student_count)
{
    struct Student *resized;
    int deleted_id;
    int i;
    int index = ask_for_student(*students, *student_count);

    if (index == -1) {
        return;
    }

    print_table_header();
    print_student_row(&(*students)[index]);
    if (!ask_yes_no("Are you sure you want to delete this student? (y/n): ")) {
        printf("Delete cancelled.\n");
        return;
    }

    deleted_id = (*students)[index].id;
    free((*students)[index].courses);

    for (i = index; i < *student_count - 1; i++) {
        (*students)[i] = (*students)[i + 1];
    }
    (*student_count)--;

    if (*student_count == 0) {
        free(*students);
        *students = NULL;
    } else {
        resized = realloc(*students, *student_count * sizeof(struct Student));
        // Shrinking almost never fails, and if it does the old, bigger block still works
        if (resized != NULL) {
            *students = resized;
        }
    }

    printf("Student %d deleted successfully.\n", deleted_id);
}

// Releases the memory of every student and their courses
void free_all_students(struct Student *students, int student_count)
{
    int i;

    for (i = 0; i < student_count; i++) {
        free(students[i].courses);
    }
    free(students);
}

// Converts text to a decimal number. Returns 1 only if the whole text is a valid number.
int parse_float(const char *text, float *value)
{
    char *end;
    double number;

    if (text[0] == '\0') {
        return 0;
    }

    number = strtod(text, &end);
    // strtod also accepts words like "nan" and "inf", which are not real marks
    if (*end != '\0' || !isfinite(number)) {
        return 0;
    }

    *value = (float) number;
    return 1;
}

// Asks for a decimal number. Returns 1 if the user typed a valid one.
int read_float(const char *prompt, float *value)
{
    char buffer[INPUT_LENGTH];

    read_line(prompt, buffer, INPUT_LENGTH);
    return parse_float(buffer, value);
}

// Copies text into destination with every letter in lowercase
void to_lowercase(const char *source, char *destination, int size)
{
    int i;

    for (i = 0; source[i] != '\0' && i < size - 1; i++) {
        destination[i] = tolower((unsigned char) source[i]);
    }
    destination[i] = '\0';
}

// Like strcmp but ignores case: negative if first comes before second, 0 if equal, positive if after
int compare_ignore_case(const char *first, const char *second)
{
    char first_lower[INPUT_LENGTH];
    char second_lower[INPUT_LENGTH];

    to_lowercase(first, first_lower, INPUT_LENGTH);
    to_lowercase(second, second_lower, INPUT_LENGTH);
    return strcmp(first_lower, second_lower);
}

// Returns 1 if two texts are the same when upper and lower case are ignored
int same_text_ignore_case(const char *first, const char *second)
{
    return compare_ignore_case(first, second) == 0;
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

// Returns 1 if the maximum marks are above zero and not unrealistically large
int is_valid_max_marks(float max_marks)
{
    return max_marks > 0 && max_marks <= MAX_MARKS_LIMIT;
}

// Returns 1 if the marks are between zero and the maximum marks
int is_valid_marks(float marks, float max_marks)
{
    return marks >= 0 && marks <= max_marks;
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

// Returns 1 if part appears anywhere inside text, ignoring upper and lower case
int contains_ignore_case(const char *text, const char *part)
{
    char text_lower[INPUT_LENGTH];
    char part_lower[INPUT_LENGTH];

    to_lowercase(text, text_lower, INPUT_LENGTH);
    to_lowercase(part, part_lower, INPUT_LENGTH);
    return strstr(text_lower, part_lower) != NULL;
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
    printf(" 0. Exit\n");
    printf("========================================\n");
}

int main(void)
{
    struct Student *students = NULL;
    int student_count = 0;
    char input[INPUT_LENGTH];
    int choice;
    int running = 1;

    while (running) {
        show_menu();

        if (!read_line("Choose an option: ", input, INPUT_LENGTH)) {
            printf("\n");
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
            case 0:
                running = 0;
                break;
            default:
                printf("Error: %d is not a menu option.\n", choice);
        }
    }

    printf("Goodbye.\n");
    free_all_students(students, student_count);
    return 0;
}
