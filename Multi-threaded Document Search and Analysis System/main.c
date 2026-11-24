#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_KEYWORD 100

// Shows how to run the program
void print_usage(const char *program_name)
{
    printf("Usage: %s \"keyword\" file1.txt [file2.txt ...]\n", program_name);
    printf("Example: %s \"IoT\" document1.txt document2.txt document3.txt\n", program_name);
}

// Returns 1 if the keyword has at least one visible character and fits in the buffer
int is_valid_keyword(const char *keyword)
{
    int i;
    int has_visible_character = 0;

    for (i = 0; keyword[i] != '\0'; i++) {
        if (!isspace((unsigned char) keyword[i])) {
            has_visible_character = 1;
        }
    }

    if (!has_visible_character) {
        printf("Error: the search keyword cannot be empty.\n");
        return 0;
    }
    if (strlen(keyword) >= MAX_KEYWORD) {
        printf("Error: the search keyword is too long (maximum %d characters).\n", MAX_KEYWORD - 1);
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[])
{
    const char *keyword;
    int file_count;

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
    return 0;
}
