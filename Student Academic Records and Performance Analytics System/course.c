#include <stdio.h>
#include <stdlib.h>

#include "input.h"
#include "student.h"
#include "course.h"

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
