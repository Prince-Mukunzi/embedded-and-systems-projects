#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define DATA_FILE "students.txt"
#define NAME_LENGTH 50
#define CODE_LENGTH 20
#define INPUT_LENGTH 100
#define PASS_PERCENT 50.0f
#define MIN_YEAR 1
#define MAX_YEAR 6

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

// Prints the column titles used by every student table
void print_table_header(void)
{
    printf("%-8s %-30s %-25s %-5s %-7s\n", "ID", "Name", "Programme", "Year", "Courses");
    printf("-----------------------------------------------------------------------------\n");
}

// Prints one student as a row of the table
void print_student_row(const struct Student *student)
{
    char full_name[NAME_LENGTH * 2];

    snprintf(full_name, sizeof(full_name), "%s %s", student->first_name, student->last_name);
    printf("%-8d %-30s %-25s %-5d %-7d\n", student->id, full_name, student->programme,
           student->year, student->course_count);
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
            case 0:
                running = 0;
                break;
            default:
                printf("Error: %d is not a menu option.\n", choice);
        }
    }

    printf("Goodbye.\n");
    free(students);
    return 0;
}
