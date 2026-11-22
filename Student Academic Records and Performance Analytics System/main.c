#include <stdio.h>

#include "records.h"
#include "input.h"
#include "student.h"
#include "course.h"
#include "search_sort.h"
#include "report.h"
#include "file_io.h"

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
