#ifndef REPORT_H
#define REPORT_H

#include "analyzer.h"

void generate_report(struct ThreadArgs *args);
int display_results(const struct ThreadArgs *thread_args, int file_count);

#endif
