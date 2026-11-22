#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>

#include "records.h"
#include "input.h"

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

// Asks a yes/no question. Returns 1 only if the user types y or Y.
int ask_yes_no(const char *question)
{
    char buffer[INPUT_LENGTH];

    read_line(question, buffer, INPUT_LENGTH);
    return strcmp(buffer, "y") == 0 || strcmp(buffer, "Y") == 0;
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

// Returns 1 if part appears anywhere inside text, ignoring upper and lower case
int contains_ignore_case(const char *text, const char *part)
{
    char text_lower[INPUT_LENGTH];
    char part_lower[INPUT_LENGTH];

    to_lowercase(text, text_lower, INPUT_LENGTH);
    to_lowercase(part, part_lower, INPUT_LENGTH);
    return strstr(text_lower, part_lower) != NULL;
}
