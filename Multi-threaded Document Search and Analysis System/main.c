#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "analyzer.h"
#include "setup.h"
#include "report.h"
#include "thread.h"

int main(int argc, char *argv[])
{
    const char *keyword;
    int file_count;
    struct ThreadArgs *thread_args;
    pthread_t *threads;
    int *thread_created;
    int failed_count;
    int i;

    if (argc < 3) {
        printf("Error: please give a search keyword and at least one file.\n");
        print_usage(argv[0]);
        return 1;
    }

    keyword = argv[1];
    file_count = argc - 2;

    if (!is_valid_keyword(keyword)) {
        print_usage(argv[0]);
        return 1;
    }

    printf("===== MULTI-THREADED DOCUMENT ANALYZER =====\n\n");
    printf("Search Keyword: %s\n", keyword);
    printf("Number of Input Files: %d\n\n", file_count);

    thread_args = calloc(file_count, sizeof(struct ThreadArgs));
    threads = calloc(file_count, sizeof(pthread_t));
    thread_created = calloc(file_count, sizeof(int));
    if (thread_args == NULL || threads == NULL || thread_created == NULL) {
        printf("Error: not enough memory to start the analysis.\n");
        free(thread_args);
        free(threads);
        free(thread_created);
        return 1;
    }

    for (i = 0; i < file_count; i++) {
        initialize_thread_args(&thread_args[i], i + 1, argv[i + 2], keyword);
    }
    check_report_conflicts(thread_args, file_count);

    for (i = 0; i < file_count; i++) {
        if (thread_args[i].status != STATUS_PENDING) {
            printf("[Thread %d] Error: %s was skipped: %s\n", i + 1, thread_args[i].filename,
                   thread_args[i].error_message);
        } else if (pthread_create(&threads[i], NULL, process_file, &thread_args[i]) != 0) {
            snprintf(thread_args[i].error_message, ERROR_LENGTH, "the thread could not be created");
            thread_args[i].status = STATUS_FAILED;
            printf("[Thread %d] Error: could not create a thread for %s\n", i + 1, thread_args[i].filename);
        } else {
            thread_created[i] = 1;
        }
    }

    // Joining a thread that was never created is not allowed, so only created ones are joined
    for (i = 0; i < file_count; i++) {
        if (thread_created[i]) {
            pthread_join(threads[i], NULL);
        }
    }

    failed_count = display_results(thread_args, file_count);

    for (i = 0; i < file_count; i++) {
        free(thread_args[i].line_numbers);
    }
    free(thread_args);
    free(threads);
    free(thread_created);

    if (failed_count > 0) {
        return 1;
    }
    return 0;
}
