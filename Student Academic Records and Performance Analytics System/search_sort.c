#include <stdio.h>

#include "input.h"
#include "student.h"
#include "search_sort.h"

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
