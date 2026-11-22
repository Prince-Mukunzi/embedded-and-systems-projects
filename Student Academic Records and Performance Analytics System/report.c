#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "input.h"
#include "student.h"
#include "course.h"
#include "report.h"

// Prints totals, overall average, best and worst student, and pass/fail counts
void print_overall_statistics(const struct Student *students, int student_count)
{
    const struct Student *highest = NULL;
    const struct Student *lowest = NULL;
    float average;
    float total_of_averages = 0.0f;
    int total_courses = 0;
    int assessed = 0;
    int passing = 0;
    int i;

    for (i = 0; i < student_count; i++) {
        total_courses += students[i].course_count;

        // Students without courses have no average, so they are left out of these numbers
        if (students[i].course_count == 0) {
            continue;
        }

        average = calculate_average(&students[i]);
        total_of_averages += average;
        assessed++;
        if (is_pass(average)) {
            passing++;
        }
        if (highest == NULL || average > calculate_average(highest)) {
            highest = &students[i];
        }
        if (lowest == NULL || average < calculate_average(lowest)) {
            lowest = &students[i];
        }
    }

    printf("Total registered students        : %d\n", student_count);
    printf("Total courses across all students: %d\n", total_courses);

    if (assessed == 0) {
        printf("No student has any courses yet, so there are no averages to report.\n");
        return;
    }

    printf("Overall average performance      : %.1f%%\n", total_of_averages / assessed);
    printf("Highest average                  : %s %s (%d) with %.1f%%\n", highest->first_name,
           highest->last_name, highest->id, calculate_average(highest));
    printf("Lowest average                   : %s %s (%d) with %.1f%%\n", lowest->first_name,
           lowest->last_name, lowest->id, calculate_average(lowest));
    printf("Students passing (average >= %.0f%%): %d\n", PASS_PERCENT, passing);
    printf("Students failing                 : %d\n", assessed - passing);
    printf("Students with no courses yet     : %d\n", student_count - assessed);
}

// Linear search: returns the position of the course code in the summary list, or -1
int find_course_summary(const struct CourseSummary *summaries, int summary_count, const char *code)
{
    int i;

    for (i = 0; i < summary_count; i++) {
        if (same_text_ignore_case(summaries[i].code, code)) {
            return i;
        }
    }
    return -1;
}

// Groups every course by its code and prints the average percentage for each one
void print_course_averages(const struct Student *students, int student_count)
{
    struct CourseSummary *summaries = NULL;
    struct CourseSummary *resized;
    const struct Course *course;
    int summary_count = 0;
    int position;
    int i;
    int j;

    for (i = 0; i < student_count; i++) {
        for (j = 0; j < students[i].course_count; j++) {
            course = &students[i].courses[j];
            position = find_course_summary(summaries, summary_count, course->code);

            if (position == -1) {
                resized = realloc(summaries, (summary_count + 1) * sizeof(struct CourseSummary));
                if (resized == NULL) {
                    printf("Error: not enough memory to build the course summary.\n");
                    free(summaries);
                    return;
                }
                summaries = resized;
                position = summary_count;
                strcpy(summaries[position].code, course->code);
                strcpy(summaries[position].name, course->name);
                summaries[position].total_percentage = 0.0f;
                summaries[position].student_count = 0;
                summary_count++;
            }

            summaries[position].total_percentage += course_percentage(course);
            summaries[position].student_count++;
        }
    }

    printf("\nAverage performance per course:\n");
    if (summary_count == 0) {
        printf("  No courses registered yet.\n");
    }
    for (i = 0; i < summary_count; i++) {
        printf("  %-10s %-30s %6.1f%%  (%d student(s))\n", summaries[i].code, summaries[i].name,
               summaries[i].total_percentage / summaries[i].student_count, summaries[i].student_count);
    }

    free(summaries);
}

// Counts how many students are in each programme of study and prints the counts
void print_programme_counts(const struct Student *students, int student_count)
{
    struct ProgrammeSummary *summaries = NULL;
    struct ProgrammeSummary *resized;
    int summary_count = 0;
    int position;
    int i;
    int j;

    for (i = 0; i < student_count; i++) {
        position = -1;
        for (j = 0; j < summary_count; j++) {
            if (same_text_ignore_case(summaries[j].programme, students[i].programme)) {
                position = j;
                break;
            }
        }

        if (position == -1) {
            resized = realloc(summaries, (summary_count + 1) * sizeof(struct ProgrammeSummary));
            if (resized == NULL) {
                printf("Error: not enough memory to build the programme summary.\n");
                free(summaries);
                return;
            }
            summaries = resized;
            position = summary_count;
            strcpy(summaries[position].programme, students[i].programme);
            summaries[position].student_count = 0;
            summary_count++;
        }

        summaries[position].student_count++;
    }

    printf("\nStudents per programme of study:\n");
    for (i = 0; i < summary_count; i++) {
        printf("  %-30s %d\n", summaries[i].programme, summaries[i].student_count);
    }

    free(summaries);
}

// Menu option 11
void generate_report(const struct Student *students, int student_count)
{
    if (student_count == 0) {
        printf("No students are registered yet, so there is nothing to report.\n");
        return;
    }

    printf("\n========== ACADEMIC PERFORMANCE REPORT ==========\n");
    print_overall_statistics(students, student_count);
    print_course_averages(students, student_count);
    print_programme_counts(students, student_count);
    printf("=================================================\n");
}
