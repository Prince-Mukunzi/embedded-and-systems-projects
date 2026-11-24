#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "analyzer.h"
#include "search.h"

// Reads one whole line of any length into a buffer that grows with realloc.
// Returns 1 if a line was read, 0 at the end of the file, -1 if memory ran out.
int read_whole_line(FILE *file, char **buffer, int *capacity)
{
    char *resized;
    int length = 0;
    int character = fgetc(file);

    if (character == EOF) {
        return 0;
    }

    while (character != EOF && character != '\n') {
        // Keep one free place for the '\0' at the end
        if (length + 1 >= *capacity) {
            resized = realloc(*buffer, *capacity * 2);
            if (resized == NULL) {
                return -1;
            }
            *buffer = resized;
            *capacity = *capacity * 2;
        }
        (*buffer)[length] = (char) character;
        length++;
        character = fgetc(file);
    }

    (*buffer)[length] = '\0';
    return 1;
}

// Changes every letter of the text to lowercase, in place
void to_lowercase(char *text)
{
    int i;

    for (i = 0; text[i] != '\0'; i++) {
        text[i] = (char) tolower((unsigned char) text[i]);
    }
}

// Counts how many times the keyword appears in the line. Both must already be lowercase.
int count_occurrences(const char *line, const char *keyword)
{
    int line_length = strlen(line);
    int keyword_length = strlen(keyword);
    int count = 0;
    int i = 0;

    while (i + keyword_length <= line_length) {
        if (strncmp(&line[i], keyword, keyword_length) == 0) {
            count++;
            // Jump past this match so the same letters are not counted twice
            i = i + keyword_length;
        } else {
            i++;
        }
    }
    return count;
}

// Adds a line number to the list of matching lines, growing the list with realloc when it is full
int record_matching_line(struct ThreadArgs *args, int line_number)
{
    int *resized;
    int new_capacity;

    if (args->matching_lines == args->line_capacity) {
        new_capacity = args->line_capacity * 2;
        if (new_capacity == 0) {
            new_capacity = START_MATCH_CAPACITY;
        }
        // realloc with a NULL pointer works like malloc, so the first call needs no special case
        resized = realloc(args->line_numbers, new_capacity * sizeof(int));
        if (resized == NULL) {
            return 0;
        }
        args->line_numbers = resized;
        args->line_capacity = new_capacity;
    }

    args->line_numbers[args->matching_lines] = line_number;
    args->matching_lines++;
    return 1;
}

// Opens the file and explains in error_message why it could not be opened
FILE *open_input_file(struct ThreadArgs *args)
{
    FILE *file = fopen(args->filename, "r");

    if (file == NULL) {
        if (errno == ENOENT) {
            snprintf(args->error_message, ERROR_LENGTH, "the file does not exist");
        } else if (errno == EACCES) {
            snprintf(args->error_message, ERROR_LENGTH, "permission denied");
        } else {
            snprintf(args->error_message, ERROR_LENGTH, "the file could not be opened");
        }
        args->status = STATUS_FAILED;
    }
    return file;
}

// Goes through the file line by line and fills in the results in args
void analyze_file(struct ThreadArgs *args)
{
    FILE *file;
    char *line;
    char keyword[MAX_KEYWORD];
    int line_capacity = START_LINE_CAPACITY;
    int result;
    int count;
    int out_of_memory = 0;

    file = open_input_file(args);
    if (file == NULL) {
        return;
    }

    line = malloc(line_capacity);
    if (line == NULL) {
        snprintf(args->error_message, ERROR_LENGTH, "not enough memory");
        args->status = STATUS_FAILED;
        fclose(file);
        return;
    }

    // Lowercasing both sides makes "IoT", "iot" and "IOT" all match
    strcpy(keyword, args->keyword);
    to_lowercase(keyword);

    result = read_whole_line(file, &line, &line_capacity);
    while (result == 1 && !out_of_memory) {
        args->total_lines++;
        to_lowercase(line);
        count = count_occurrences(line, keyword);
        if (count > 0) {
            args->occurrences = args->occurrences + count;
            if (!record_matching_line(args, args->total_lines)) {
                out_of_memory = 1;
            }
        }
        if (!out_of_memory) {
            result = read_whole_line(file, &line, &line_capacity);
        }
    }

    if (result == -1 || out_of_memory) {
        snprintf(args->error_message, ERROR_LENGTH, "not enough memory to read a line");
        args->status = STATUS_FAILED;
    } else if (ferror(file)) {
        // A directory or a disk problem shows up here as a read error, not as an open error
        snprintf(args->error_message, ERROR_LENGTH, "the file could not be read");
        args->status = STATUS_FAILED;
    } else {
        args->status = STATUS_COMPLETED;
    }

    free(line);
    fclose(file);
}
