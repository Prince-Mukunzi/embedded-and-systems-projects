#ifndef RECORDS_H
#define RECORDS_H

#define DATA_FILE "students.txt"
#define NAME_LENGTH 50
#define CODE_LENGTH 20
#define INPUT_LENGTH 100
#define PASS_PERCENT 50.0f
#define MIN_YEAR 1
#define MAX_YEAR 6
#define MAX_MARKS_LIMIT 1000.0f
#define FILE_LINE_LENGTH 256

struct Course {
    char code[CODE_LENGTH];
    char name[NAME_LENGTH];
    float marks;
    float max_marks;
};

struct Student {
    int id;
    char first_name[NAME_LENGTH];
    char last_name[NAME_LENGTH];
    char programme[NAME_LENGTH];
    int year;
    int course_count;
    struct Course *courses;
};

#endif
