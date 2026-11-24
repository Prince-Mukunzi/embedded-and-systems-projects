#include <stdio.h>

#include "analyzer.h"
#include "search.h"
#include "report.h"
#include "thread.h"

// The thread function: each thread analyses one file and writes its report.
// Threads share nothing, because each one only touches its own ThreadArgs, so no mutex is needed.
void *process_file(void *arg)
{
    struct ThreadArgs *args = (struct ThreadArgs *) arg;

    printf("[Thread %d] Processing %s ...\n", args->thread_number, args->filename);

    analyze_file(args);
    generate_report(args);

    if (args->status == STATUS_COMPLETED) {
        printf("[Thread %d] Successfully processed %s\n", args->thread_number, args->filename);
    } else {
        printf("[Thread %d] Error processing %s: %s\n", args->thread_number, args->filename, args->error_message);
    }
    return NULL;
}
