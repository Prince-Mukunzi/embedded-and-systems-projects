#ifndef SEARCH_H
#define SEARCH_H

#include <stdio.h>
#include "analyzer.h"

int read_whole_line(FILE *file, char **buffer, int *capacity);
void to_lowercase(char *text);
int count_occurrences(const char *line, const char *keyword);
int record_matching_line(struct ThreadArgs *args, int line_number);
FILE *open_input_file(struct ThreadArgs *args);
void analyze_file(struct ThreadArgs *args);

#endif
