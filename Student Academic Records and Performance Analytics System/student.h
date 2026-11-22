#ifndef STUDENT_H
#define STUDENT_H

#include "records.h"

int find_student_index(const struct Student *students, int student_count, int id);
void register_student(struct Student **students, int *student_count);
float calculate_average(const struct Student *student);
void print_table_header(void);
void print_student_row(const struct Student *student);
void display_all_students(const struct Student *students, int student_count);
int ask_for_student(const struct Student *students, int student_count);
void display_student_details(const struct Student *student);
void view_student(const struct Student *students, int student_count);
int update_text_field(const char *label, char *field, int size);
void update_student(struct Student *students, int student_count);
void delete_student(struct Student **students, int *student_count);
void free_all_students(struct Student *students, int student_count);

#endif
