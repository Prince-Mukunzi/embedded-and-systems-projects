#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "analyzer.h"
#include "setup.h"

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

// Marks a file as failed when its report would clash with another file:
// two threads writing one report, or one thread reading a report another thread is writing
void check_report_conflicts(struct ThreadArgs *thread_args, int file_count)
{
    int i;
    int j;

    for (i = 0; i < file_count; i++) {
        for (j = 0; j < file_count; j++) {
            if (i == j || thread_args[i].status != STATUS_PENDING) {
                continue;
            }
            if (j < i && strcmp(thread_args[i].report_name, thread_args[j].report_name) == 0) {
                snprintf(thread_args[i].error_message, ERROR_LENGTH, "its report %s would overwrite the report of file %d",
                         thread_args[i].report_name, j + 1);
                thread_args[i].status = STATUS_FAILED;
            } else if (strcmp(thread_args[i].filename, thread_args[j].report_name) == 0) {
                snprintf(thread_args[i].error_message, ERROR_LENGTH, "it is the report file that file %d is writing",
                         j + 1);
                thread_args[i].status = STATUS_FAILED;
            }
        }
    }
}
