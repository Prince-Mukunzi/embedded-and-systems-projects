#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>

#include "analyzer.h"

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

// Writes the results for one file into its own report file
void generate_report(struct ThreadArgs *args)
{
    FILE *report;
    int i;
    int write_failed;

    report = fopen(args->report_name, "w");
    if (report == NULL) {
        // Keep the first error if the analysis had already failed
        if (args->status != STATUS_FAILED) {
            snprintf(args->error_message, ERROR_LENGTH, "the report file %s could not be created", args->report_name);
            args->status = STATUS_FAILED;
        }
        return;
    }

    fprintf(report, "===== DOCUMENT ANALYSIS REPORT =====\n");
    fprintf(report, "Input file            : %s\n", args->filename);
    fprintf(report, "Search keyword        : %s\n", args->keyword);
    fprintf(report, "Total lines processed : %d\n", args->total_lines);
    fprintf(report, "Keyword occurrences   : %d\n", args->occurrences);
    fprintf(report, "Matching lines        : %d\n", args->matching_lines);

    fprintf(report, "Line numbers          : ");
    if (args->matching_lines == 0) {
        fprintf(report, "none");
    }
    for (i = 0; i < args->matching_lines; i++) {
        if (i > 0) {
            fprintf(report, ", ");
        }
        fprintf(report, "%d", args->line_numbers[i]);
    }
    fprintf(report, "\n");

    if (args->status == STATUS_FAILED) {
        fprintf(report, "Status                : Failed (%s)\n", args->error_message);
    } else {
        fprintf(report, "Status                : Success\n");
    }

    // ferror is checked before fclose so a full disk is not reported as success
    write_failed = ferror(report);
    if (fclose(report) != 0) {
        write_failed = 1;
    }
    if (write_failed) {
        if (args->status != STATUS_FAILED) {
            snprintf(args->error_message, ERROR_LENGTH, "the report file %s could not be written", args->report_name);
            args->status = STATUS_FAILED;
        }
        return;
    }
    args->report_written = 1;
}

// The thread function: each thread analyses one file and writes its report.
// Threads share nothing, because each one only touches its own ThreadArgs, so no mutex is needed.
void *process_file(void *arg)
{
    struct ThreadArgs *args = (struct ThreadArgs *) arg;

    printf("[Thread %d] Processing %s ...\n", args->thread_number, args->filename);

    analyze_file(args);
    generate_report(args);

    if (args->status == STATUS_COMPLETED) {
        printf("[Thread %d] Successfully processed %s\n", args->thread_number, args->filename);
    } else {
        printf("[Thread %d] Error processing %s: %s\n", args->thread_number, args->filename, args->error_message);
    }
    return NULL;
}

// Prints the summary after all threads have finished. Returns how many files failed.
int display_results(const struct ThreadArgs *thread_args, int file_count)
{
    int failed_count = 0;
    int report_count = 0;
    int i;

    printf("\n===== PROCESSING SUMMARY =====\n\n");
    for (i = 0; i < file_count; i++) {
        if (thread_args[i].status == STATUS_COMPLETED) {
            printf("%s: Completed (%d occurrence(s) on %d line(s), report: %s)\n", thread_args[i].filename,
                   thread_args[i].occurrences, thread_args[i].matching_lines, thread_args[i].report_name);
        } else {
            printf("%s: Failed (%s)\n", thread_args[i].filename, thread_args[i].error_message);
            failed_count++;
        }
        if (thread_args[i].report_written) {
            report_count++;
        }
    }

    printf("\n");
    if (failed_count == 0) {
        printf("All processing tasks completed.\n");
        printf("Reports generated successfully.\n");
    } else {
        printf("All processing tasks finished. %d of %d file(s) failed.\n", failed_count, file_count);
        printf("%d of %d report(s) generated.\n", report_count, file_count);
    }
    return failed_count;
}

int main(int argc, char *argv[])
{
    const char *keyword;
    int file_count;
    struct ThreadArgs *thread_args;
    pthread_t *threads;
    int *thread_created;
    int failed_count;
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
    threads = calloc(file_count, sizeof(pthread_t));
    thread_created = calloc(file_count, sizeof(int));
    if (thread_args == NULL || threads == NULL || thread_created == NULL) {
        printf("Error: not enough memory to start the analysis.\n");
        free(thread_args);
        free(threads);
        free(thread_created);
        return 1;
    }

    for (i = 0; i < file_count; i++) {
        initialize_thread_args(&thread_args[i], i + 1, argv[i + 2], keyword);
    }
    check_report_conflicts(thread_args, file_count);

    for (i = 0; i < file_count; i++) {
        if (thread_args[i].status != STATUS_PENDING) {
            printf("[Thread %d] Error: %s was skipped: %s\n", i + 1, thread_args[i].filename,
                   thread_args[i].error_message);
        } else if (pthread_create(&threads[i], NULL, process_file, &thread_args[i]) != 0) {
            snprintf(thread_args[i].error_message, ERROR_LENGTH, "the thread could not be created");
            thread_args[i].status = STATUS_FAILED;
            printf("[Thread %d] Error: could not create a thread for %s\n", i + 1, thread_args[i].filename);
        } else {
            thread_created[i] = 1;
        }
    }

    // Joining a thread that was never created is not allowed, so only created ones are joined
    for (i = 0; i < file_count; i++) {
        if (thread_created[i]) {
            pthread_join(threads[i], NULL);
        }
    }

    failed_count = display_results(thread_args, file_count);

    for (i = 0; i < file_count; i++) {
        free(thread_args[i].line_numbers);
    }
    free(thread_args);
    free(threads);
    free(thread_created);

    if (failed_count > 0) {
        return 1;
    }
    return 0;
}
