#ifndef ANALYZER_H
#define ANALYZER_H

#define MAX_KEYWORD 100
#define MAX_FILENAME 256
#define REPORT_NAME_LENGTH 300
#define ERROR_LENGTH 400
#define START_LINE_CAPACITY 128
#define START_MATCH_CAPACITY 8
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

#endif
