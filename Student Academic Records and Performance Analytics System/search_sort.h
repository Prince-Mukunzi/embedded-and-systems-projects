#ifndef SEARCH_SORT_H
#define SEARCH_SORT_H

#include "records.h"

void search_by_id(const struct Student *students, int student_count);
void search_by_text(const struct Student *students, int student_count, int search_type);
void search_students(const struct Student *students, int student_count);
int should_swap(const struct Student *a, const struct Student *b, int sort_type);
void bubble_sort_students(struct Student *students, int student_count, int sort_type);
void sort_students(struct Student *students, int student_count);

#endif
