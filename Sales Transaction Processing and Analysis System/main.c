#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define DATA_FILE "transactions.txt"
#define PRODUCT_LENGTH 50
#define DATE_LENGTH 11
#define INPUT_LENGTH 100
#define INITIAL_CAPACITY 4

struct Transaction {
    int id;
    char product[PRODUCT_LENGTH];
    int quantity;
    float unit_price;
    float total;
    char date[DATE_LENGTH];
};

struct TransactionList {
    struct Transaction *items;
    int count;
    int capacity;
};

// Reads one line from the keyboard without the newline. Returns 0 when input has ended.
int read_line(const char *prompt, char *buffer, int size)
{
    int length;
    int c;

    printf("%s", prompt);
    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
        return 0;
    }

    length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    } else {
        // The line was longer than the buffer, so the rest of it is thrown away
        while ((c = getchar()) != '\n' && c != EOF) {
        }
    }
    return 1;
}

// Converts text to a whole number. Returns 1 only if the whole text is a valid number.
int parse_int(const char *text, int *value)
{
    char *end;
    long number;

    if (text[0] == '\0') {
        return 0;
    }

    number = strtol(text, &end, 10);
    if (*end != '\0' || number < INT_MIN || number > INT_MAX) {
        return 0;
    }

    *value = (int) number;
    return 1;
}

// Asks for a whole number. Returns 1 if the user typed a valid one.
int read_int(const char *prompt, int *value)
{
    char buffer[INPUT_LENGTH];

    read_line(prompt, buffer, INPUT_LENGTH);
    return parse_int(buffer, value);
}

// Creates an empty list with room for a few transactions. Returns 0 if memory runs out.
int create_list(struct TransactionList *list)
{
    list->items = calloc(INITIAL_CAPACITY, sizeof(struct Transaction));
    if (list->items == NULL) {
        return 0;
    }
    list->count = 0;
    list->capacity = INITIAL_CAPACITY;
    return 1;
}

// Releases the memory used by the list
void free_list(struct TransactionList *list)
{
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

// Prints the main menu
void show_menu(void)
{
    printf("\n===== SALES TRANSACTION MANAGEMENT SYSTEM =====\n");
    printf(" 0. Exit\n");
}

int main(void)
{
    struct TransactionList list;
    char input[INPUT_LENGTH];
    int choice;
    int running = 1;

    if (!create_list(&list)) {
        printf("Error: not enough memory to start the program.\n");
        return 1;
    }

    while (running) {
        show_menu();

        if (!read_line("Choose an option: ", input, INPUT_LENGTH)) {
            printf("\n");
            break;
        }

        if (!parse_int(input, &choice)) {
            printf("Error: please enter a number from the menu.\n");
            continue;
        }

        switch (choice) {
            case 0:
                running = 0;
                break;
            default:
                printf("Error: %d is not a menu option.\n", choice);
        }
    }

    printf("Goodbye.\n");
    free_list(&list);
    return 0;
}
