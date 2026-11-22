#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdio.h>
#include "records.h"

int save_records(const struct Student *students, int student_count, const char *filename);
int read_file_line(FILE *file, char *line, int *line_number);
int parse_student_line(const char *line, struct Student *student);
int parse_course_line(const char *line, struct Course *course);
int load_one_student(FILE *file, struct Student *student, int *line_number);
int load_records(struct Student **students, int *student_count, const char *filename);

#endif
