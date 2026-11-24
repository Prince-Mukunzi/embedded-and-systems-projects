#ifndef REPORT_H
#define REPORT_H

#include "records.h"

struct CourseSummary {
    char code[CODE_LENGTH];
    char name[NAME_LENGTH];
    float total_percentage;
    int student_count;
};

struct ProgrammeSummary {
    char programme[NAME_LENGTH];
    int student_count;
};

void print_overall_statistics(const struct Student *students, int student_count);
int find_course_summary(const struct CourseSummary *summaries, int summary_count, const char *code);
void print_course_averages(const struct Student *students, int student_count);
void print_programme_counts(const struct Student *students, int student_count);
void generate_report(const struct Student *students, int student_count);

#endif
