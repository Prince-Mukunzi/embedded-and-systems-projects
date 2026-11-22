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

// Prints the main menu
void show_menu(void)
{
    printf("\n========================================\n");
    printf("  STUDENT ACADEMIC RECORDS SYSTEM\n");
    printf("========================================\n");
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
            case 0:
                running = 0;
                break;
            default:
                printf("Error: %d is not a menu option.\n", choice);
        }
    }

    printf("Goodbye.\n");
    (void) students;
    (void) student_count;
    return 0;
}
