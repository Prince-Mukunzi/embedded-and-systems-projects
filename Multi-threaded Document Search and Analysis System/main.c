#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define MAX_KEYWORD 100
#define MAX_FILENAME 256
#define REPORT_NAME_LENGTH 300
#define ERROR_LENGTH 100
#define START_LINE_CAPACITY 128
#define STATUS_PENDING 0
#define STATUS_COMPLETED 1
#define STATUS_FAILED 2

struct ThreadArgs {
    int thread_number;
    char filename[MAX_FILENAME];
    char keyword[MAX_KEYWORD];
    char report_name[REPORT_NAME_LENGTH];
    int total_lines;
    int occurrences;
    int matching_lines;
    int *line_numbers;
    int line_capacity;
    int status;
    int report_written;
    char error_message[ERROR_LENGTH];
};

// Shows how to run the program
void print_usage(const char *program_name)
{
    printf("Usage: %s \"keyword\" file1.txt [file2.txt ...]\n", program_name);
    printf("Example: %s \"IoT\" document1.txt document2.txt document3.txt\n", program_name);
}

// Returns 1 if the keyword has at least one visible character and fits in the buffer
int is_valid_keyword(const char *keyword)
{
    int i;
    int has_visible_character = 0;

    for (i = 0; keyword[i] != '\0'; i++) {
        if (!isspace((unsigned char) keyword[i])) {
            has_visible_character = 1;
        }
    }

    if (!has_visible_character) {
        printf("Error: the search keyword cannot be empty.\n");
        return 0;
    }
    if (strlen(keyword) >= MAX_KEYWORD) {
        printf("Error: the search keyword is too long (maximum %d characters).\n", MAX_KEYWORD - 1);
        return 0;
    }
    return 1;
}

// Builds the report name from the file name: "notes/document1.txt" becomes "document1_report.txt"
void make_report_name(const char *filename, char *report_name, int size)
{
    char base_name[MAX_FILENAME];
    const char *last_slash = strrchr(filename, '/');
    char *last_dot;

    if (last_slash != NULL) {
        filename = last_slash + 1;
    }
    strncpy(base_name, filename, MAX_FILENAME - 1);
    base_name[MAX_FILENAME - 1] = '\0';

    // A dot at position 0 is a hidden file like ".notes", not an extension
    last_dot = strrchr(base_name, '.');
    if (last_dot != NULL && last_dot != base_name) {
        *last_dot = '\0';
    }
    snprintf(report_name, size, "%s_report.txt", base_name);
}

// Fills in the starting values for the thread that will process one file
void initialize_thread_args(struct ThreadArgs *args, int thread_number, const char *filename, const char *keyword)
{
    args->thread_number = thread_number;
    strncpy(args->keyword, keyword, MAX_KEYWORD - 1);
    args->keyword[MAX_KEYWORD - 1] = '\0';
    args->total_lines = 0;
    args->occurrences = 0;
    args->matching_lines = 0;
    args->line_numbers = NULL;
    args->line_capacity = 0;
    args->status = STATUS_PENDING;
    args->report_written = 0;
    args->error_message[0] = '\0';

    strncpy(args->filename, filename, MAX_FILENAME - 1);
    args->filename[MAX_FILENAME - 1] = '\0';
    if (strlen(filename) >= MAX_FILENAME) {
        args->status = STATUS_FAILED;
        snprintf(args->error_message, ERROR_LENGTH, "the file name is too long");
    }
    make_report_name(args->filename, args->report_name, REPORT_NAME_LENGTH);
}

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
    int line_capacity = START_LINE_CAPACITY;
    int result;

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

    result = read_whole_line(file, &line, &line_capacity);
    while (result == 1) {
        args->total_lines++;
        result = read_whole_line(file, &line, &line_capacity);
    }

    // A directory or a disk problem shows up as a read error, not as an open error
    if (result == -1) {
        snprintf(args->error_message, ERROR_LENGTH, "not enough memory to read a line");
        args->status = STATUS_FAILED;
    } else if (ferror(file)) {
        snprintf(args->error_message, ERROR_LENGTH, "the file could not be read");
        args->status = STATUS_FAILED;
    } else {
        args->status = STATUS_COMPLETED;
    }

    free(line);
    fclose(file);
}

int main(int argc, char *argv[])
{
    const char *keyword;
    int file_count;
    struct ThreadArgs *thread_args;
    int i;

    if (argc < 3) {
        printf("Error: please give a search keyword and at least one file.\n");
        print_usage(argv[0]);
        return 1;
    }

    keyword = argv[1];
    file_count = argc - 2;

    if (!is_valid_keyword(keyword)) {
        print_usage(argv[0]);
        return 1;
    }

    printf("===== MULTI-THREADED DOCUMENT ANALYZER =====\n\n");
    printf("Search Keyword: %s\n", keyword);
    printf("Number of Input Files: %d\n\n", file_count);

    thread_args = calloc(file_count, sizeof(struct ThreadArgs));
    if (thread_args == NULL) {
        printf("Error: not enough memory to start the analysis.\n");
        return 1;
    }

    for (i = 0; i < file_count; i++) {
        initialize_thread_args(&thread_args[i], i + 1, argv[i + 2], keyword);
        if (thread_args[i].status == STATUS_PENDING) {
            analyze_file(&thread_args[i]);
        }
        printf("[Thread %d] %s: %d lines, status %d %s\n", thread_args[i].thread_number, thread_args[i].filename,
               thread_args[i].total_lines, thread_args[i].status, thread_args[i].error_message);
    }

    free(thread_args);
    return 0;
}
