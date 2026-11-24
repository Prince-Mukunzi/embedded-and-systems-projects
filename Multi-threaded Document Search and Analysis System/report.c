#include <stdio.h>

#include "analyzer.h"
#include "report.h"

// Writes the results for one file into its own report file
void generate_report(struct ThreadArgs *args)
{
    FILE *report;
    int i;
    int write_failed;

    report = fopen(args->report_name, "w");
    if (report == NULL) {
        // Keep the first error if the analysis had already failed
        if (args->status != STATUS_FAILED) {
            snprintf(args->error_message, ERROR_LENGTH, "the report file %s could not be created", args->report_name);
            args->status = STATUS_FAILED;
        }
        return;
    }

    fprintf(report, "===== DOCUMENT ANALYSIS REPORT =====\n");
    fprintf(report, "Input file            : %s\n", args->filename);
    fprintf(report, "Search keyword        : %s\n", args->keyword);
    fprintf(report, "Total lines processed : %d\n", args->total_lines);
    fprintf(report, "Keyword occurrences   : %d\n", args->occurrences);
    fprintf(report, "Matching lines        : %d\n", args->matching_lines);

    fprintf(report, "Line numbers          : ");
    if (args->matching_lines == 0) {
        fprintf(report, "none");
    }
    for (i = 0; i < args->matching_lines; i++) {
        if (i > 0) {
            fprintf(report, ", ");
        }
        fprintf(report, "%d", args->line_numbers[i]);
    }
    fprintf(report, "\n");

    if (args->status == STATUS_FAILED) {
        fprintf(report, "Status                : Failed (%s)\n", args->error_message);
    } else {
        fprintf(report, "Status                : Success\n");
    }

    // ferror is checked before fclose so a full disk is not reported as success
    write_failed = ferror(report);
    if (fclose(report) != 0) {
        write_failed = 1;
    }
    if (write_failed) {
        if (args->status != STATUS_FAILED) {
            snprintf(args->error_message, ERROR_LENGTH, "the report file %s could not be written", args->report_name);
            args->status = STATUS_FAILED;
        }
        return;
    }
    args->report_written = 1;
}

// Prints the summary after all threads have finished. Returns how many files failed.
int display_results(const struct ThreadArgs *thread_args, int file_count)
{
    int failed_count = 0;
    int report_count = 0;
    int i;

    printf("\n===== PROCESSING SUMMARY =====\n\n");
    for (i = 0; i < file_count; i++) {
        if (thread_args[i].status == STATUS_COMPLETED) {
            printf("%s: Completed (%d occurrence(s) on %d line(s), report: %s)\n", thread_args[i].filename,
                   thread_args[i].occurrences, thread_args[i].matching_lines, thread_args[i].report_name);
        } else {
            printf("%s: Failed (%s)\n", thread_args[i].filename, thread_args[i].error_message);
            failed_count++;
        }
        if (thread_args[i].report_written) {
            report_count++;
        }
    }

    printf("\n");
    if (failed_count == 0) {
        printf("All processing tasks completed.\n");
        printf("Reports generated successfully.\n");
    } else {
        printf("All processing tasks finished. %d of %d file(s) failed.\n", failed_count, file_count);
        printf("%d of %d report(s) generated.\n", report_count, file_count);
    }
    return failed_count;
}
