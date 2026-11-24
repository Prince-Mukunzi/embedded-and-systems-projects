#ifndef SETUP_H
#define SETUP_H

#include "analyzer.h"

void print_usage(const char *program_name);
int is_valid_keyword(const char *keyword);
void make_report_name(const char *filename, char *report_name, int size);
void initialize_thread_args(struct ThreadArgs *args, int thread_number, const char *filename, const char *keyword);
void check_report_conflicts(struct ThreadArgs *thread_args, int file_count);

#endif
