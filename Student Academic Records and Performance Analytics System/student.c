#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "input.h"
#include "student.h"

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
