#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "input.h"
#include "student.h"
#include "course.h"
#include "file_io.h"

// Writes every student and their courses to a text file. Returns 1 on success.
int save_records(const struct Student *students, int student_count, const char *filename)
{
    FILE *file;
    const struct Student *student;
    const struct Course *course;
    int write_failed;
    int i;
    int j;

    file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: could not open %s for writing.\n", filename);
        return 0;
    }

    // Line 1 is the number of students, then each student line is followed by its course lines
    fprintf(file, "%d\n", student_count);
    for (i = 0; i < student_count; i++) {
        student = &students[i];
        fprintf(file, "%d|%s|%s|%s|%d|%d\n", student->id, student->first_name, student->last_name,
                student->programme, student->year, student->course_count);

        for (j = 0; j < student->course_count; j++) {
            course = &student->courses[j];
            fprintf(file, "%s|%s|%.2f|%.2f\n", course->code, course->name, course->marks, course->max_marks);
        }
    }

    // fclose also writes out anything still waiting in memory, so its result must be checked
    write_failed = ferror(file);
    if (fclose(file) != 0 || write_failed) {
        printf("Error: something went wrong while writing %s.\n", filename);
        return 0;
    }

    printf("%d student record(s) saved to %s.\n", student_count, filename);
    return 1;
}

// Reads the next line of the file and counts it. Returns 0 when the file has ended.
int read_file_line(FILE *file, char *line, int *line_number)
{
    if (fgets(line, FILE_LINE_LENGTH, file) == NULL) {
        return 0;
    }
    (*line_number)++;
    return 1;
}

// Fills a student from one saved line. Returns 1 only if every value is present and valid.
int parse_student_line(const char *line, struct Student *student)
{
    int fields = sscanf(line, "%d|%49[^|]|%49[^|]|%49[^|]|%d|%d", &student->id, student->first_name,
                        student->last_name, student->programme, &student->year, &student->course_count);

    return fields == 6 && student->id > 0 && is_valid_year(student->year) && student->course_count >= 0;
}

// Fills a course from one saved line. Returns 1 only if every value is present and valid.
int parse_course_line(const char *line, struct Course *course)
{
    int fields = sscanf(line, "%19[^|]|%49[^|]|%f|%f", course->code, course->name, &course->marks,
                        &course->max_marks);

    return fields == 4 && is_valid_max_marks(course->max_marks) && is_valid_marks(course->marks, course->max_marks);
}

// Reads one student line and the course lines under it. Returns 0 if anything is missing or invalid.
int load_one_student(FILE *file, struct Student *student, int *line_number)
{
    char line[FILE_LINE_LENGTH];
    int j;

    if (!read_file_line(file, line, line_number) || !parse_student_line(line, student)) {
        student->course_count = 0;
        return 0;
    }
    if (student->course_count == 0) {
        return 1;
    }

    student->courses = calloc(student->course_count, sizeof(struct Course));
    if (student->courses == NULL) {
        printf("Error: not enough memory to load the courses.\n");
        student->course_count = 0;
        return 0;
    }

    for (j = 0; j < student->course_count; j++) {
        if (!read_file_line(file, line, line_number) || !parse_course_line(line, &student->courses[j])) {
            return 0;
        }
    }
    return 1;
}

// Reads all records from the file into memory. Returns 0 if the file exists but cannot be loaded.
int load_records(struct Student **students, int *student_count, const char *filename)
{
    FILE *file;
    struct Student *loaded = NULL;
    char line[FILE_LINE_LENGTH];
    int line_number = 0;
    int total = 0;
    int ok = 1;
    int i;

    file = fopen(filename, "r");
    if (file == NULL) {
        if (errno == ENOENT) {
            printf("No saved records found. Starting with an empty list.\n");
            return 1;
        }
        printf("Error: %s exists but could not be opened.\n", filename);
        return 0;
    }

    if (!read_file_line(file, line, &line_number) || sscanf(line, "%d", &total) != 1 || total < 0) {
        ok = 0;
        total = 0;
    }

    // calloc sets every course pointer to NULL, so a half-loaded list can always be freed safely
    if (ok && total > 0) {
        loaded = calloc(total, sizeof(struct Student));
        if (loaded == NULL) {
            printf("Error: not enough memory to load the records.\n");
            ok = 0;
            total = 0;
        }
    }

    for (i = 0; ok && i < total; i++) {
        if (!load_one_student(file, &loaded[i], &line_number)) {
            ok = 0;
        } else if (find_student_index(loaded, i, loaded[i].id) != -1) {
            printf("Error: student ID %d appears twice in the file.\n", loaded[i].id);
            ok = 0;
        }
    }
    fclose(file);

    if (!ok) {
        printf("Error: %s could not be loaded (problem near line %d).\n", filename, line_number);
        free_all_students(loaded, total);
        return 0;
    }

    *students = loaded;
    *student_count = total;
    printf("%d student record(s) loaded from %s.\n", total, filename);
    return 1;
}
