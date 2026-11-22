#ifndef COURSE_H
#define COURSE_H

#include "records.h"

float course_percentage(const struct Course *course);
int is_pass(float percentage);
int find_course_index(const struct Student *student, const char *code);
void add_course(struct Student *students, int student_count);
void update_course_marks(struct Student *students, int student_count);
void display_performance(const struct Student *students, int student_count);

#endif
