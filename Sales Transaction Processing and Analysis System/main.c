#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>

#define DATA_FILE "transactions.txt"
#define PRODUCT_LENGTH 50
#define DATE_LENGTH 11
#define INPUT_LENGTH 100
#define INITIAL_CAPACITY 4
#define MAX_QUANTITY 1000000
#define MAX_PRICE 1000000.0f
#define MIN_YEAR 2000
#define MAX_YEAR 2100

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

// Converts text to a decimal number. Returns 1 only if the whole text is a valid number.
int parse_float(const char *text, float *value)
{
    char *end;
    double number;

    if (text[0] == '\0') {
        return 0;
    }

    number = strtod(text, &end);
    // strtod also accepts words like "nan" and "inf", which are not real prices
    if (*end != '\0' || !isfinite(number)) {
        return 0;
    }

    *value = (float) number;
    return 1;
}

// Asks for a decimal number. Returns 1 if the user typed a valid one.
int read_float(const char *prompt, float *value)
{
    char buffer[INPUT_LENGTH];

    read_line(prompt, buffer, INPUT_LENGTH);
    return parse_float(buffer, value);
}

// Returns 1 if the text has only spaces or tabs in it
int is_blank(const char *text)
{
    int i;

    for (i = 0; text[i] != '\0'; i++) {
        if (!isspace((unsigned char) text[i])) {
            return 0;
        }
    }
    return 1;
}

// Checks that text is not empty, fits in the field and has no '|' (the save file separator)
int is_valid_text(const char *text, int size)
{
    if (is_blank(text)) {
        printf("Error: this field cannot be empty.\n");
        return 0;
    }
    if ((int) strlen(text) >= size) {
        printf("Error: the text is too long (maximum %d characters).\n", size - 1);
        return 0;
    }
    if (strchr(text, '|') != NULL) {
        printf("Error: the '|' character is not allowed.\n");
        return 0;
    }
    return 1;
}

// Asks for a text field and copies it into destination only if it is valid
int read_text(const char *prompt, char *destination, int size)
{
    char buffer[INPUT_LENGTH];

    read_line(prompt, buffer, INPUT_LENGTH);
    if (!is_valid_text(buffer, size)) {
        return 0;
    }

    strcpy(destination, buffer);
    return 1;
}

// Returns 1 for leap years, when February has 29 days
int is_leap_year(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

// Returns 1 if the text is a real calendar date written as YYYY-MM-DD
int is_valid_date(const char *date)
{
    int days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int year;
    int month;
    int day;
    int i;

    if (strlen(date) != 10 || date[4] != '-' || date[7] != '-') {
        return 0;
    }
    for (i = 0; i < 10; i++) {
        if (i != 4 && i != 7 && !isdigit((unsigned char) date[i])) {
            return 0;
        }
    }

    sscanf(date, "%d-%d-%d", &year, &month, &day);
    if (year < MIN_YEAR || year > MAX_YEAR || month < 1 || month > 12) {
        return 0;
    }
    if (month == 2 && is_leap_year(year)) {
        days_in_month[1] = 29;
    }
    return day >= 1 && day <= days_in_month[month - 1];
}

// Returns 1 if the quantity is a positive whole number within the limit
int is_valid_quantity(int quantity)
{
    return quantity > 0 && quantity <= MAX_QUANTITY;
}

// Returns 1 if the unit price is above zero and within the limit
int is_valid_price(float price)
{
    return price > 0 && price <= MAX_PRICE;
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

// Doubles the capacity of the list when it is full. Returns 0 if memory runs out.
int grow_list(struct TransactionList *list)
{
    struct Transaction *resized;
    int new_capacity = list->capacity * 2;

    resized = realloc(list->items, new_capacity * sizeof(struct Transaction));
    // On failure the old list is still valid, so nothing is lost
    if (resized == NULL) {
        return 0;
    }

    list->items = resized;
    list->capacity = new_capacity;
    return 1;
}

// Linear search: returns the position of the transaction with this ID, or -1 if not found
int find_transaction_index(const struct TransactionList *list, int id)
{
    int i;

    for (i = 0; i < list->count; i++) {
        if (list->items[i].id == id) {
            return i;
        }
    }
    return -1;
}

// Menu option 1: asks for the details of a sale and adds it to the list
void add_transaction(struct TransactionList *list)
{
    struct Transaction new_transaction;

    if (!read_int("Transaction ID: ", &new_transaction.id) || new_transaction.id <= 0) {
        printf("Error: the transaction ID must be a positive whole number.\n");
        return;
    }
    if (find_transaction_index(list, new_transaction.id) != -1) {
        printf("Error: a transaction with ID %d already exists.\n", new_transaction.id);
        return;
    }
    if (!read_text("Product name: ", new_transaction.product, PRODUCT_LENGTH)) {
        return;
    }
    if (!read_int("Quantity sold: ", &new_transaction.quantity) || !is_valid_quantity(new_transaction.quantity)) {
        printf("Error: the quantity must be a whole number from 1 to %d.\n", MAX_QUANTITY);
        return;
    }
    if (!read_float("Unit price: ", &new_transaction.unit_price) || !is_valid_price(new_transaction.unit_price)) {
        printf("Error: the unit price must be a number above 0 and up to %.0f.\n", MAX_PRICE);
        return;
    }
    if (!read_text("Transaction date (YYYY-MM-DD): ", new_transaction.date, DATE_LENGTH)
        || !is_valid_date(new_transaction.date)) {
        printf("Error: the date must be a real date written as YYYY-MM-DD.\n");
        return;
    }

    new_transaction.total = new_transaction.quantity * new_transaction.unit_price;

    if (list->count == list->capacity && !grow_list(list)) {
        printf("Error: not enough memory to add the transaction.\n");
        return;
    }

    list->items[list->count] = new_transaction;
    list->count++;
    printf("Transaction %d added. Total amount: %.2f\n", new_transaction.id, new_transaction.total);
}

// Prints the column titles used by every transaction table
void print_table_header(void)
{
    printf("%-8s %-30s %8s %12s %14s %-10s\n", "ID", "Product", "Quantity", "Unit price", "Total", "Date");
    printf("-------------------------------------------------------------------------------------\n");
}

// Prints one transaction as a row of the table
void print_transaction(const struct Transaction *transaction)
{
    printf("%-8d %-30s %8d %12.2f %14.2f %-10s\n", transaction->id, transaction->product,
           transaction->quantity, transaction->unit_price, transaction->total, transaction->date);
}

// Menu option 2: prints every recorded transaction
void display_all_transactions(const struct TransactionList *list)
{
    int i;

    if (list->count == 0) {
        printf("No transactions have been recorded yet.\n");
        return;
    }

    print_table_header();
    for (i = 0; i < list->count; i++) {
        print_transaction(&list->items[i]);
    }
    printf("Total: %d transaction(s)\n", list->count);
}

// Asks for a transaction ID and returns its position, or -1 with an error message
int ask_for_transaction(const struct TransactionList *list)
{
    int id;
    int index;

    if (list->count == 0) {
        printf("No transactions have been recorded yet.\n");
        return -1;
    }
    if (!read_int("Enter the transaction ID: ", &id)) {
        printf("Error: the transaction ID must be a whole number.\n");
        return -1;
    }

    index = find_transaction_index(list, id);
    if (index == -1) {
        printf("Error: no transaction has ID %d.\n", id);
    }
    return index;
}

// Menu option 3: Enter keeps the current value of a field
void update_transaction(struct TransactionList *list)
{
    struct Transaction updated;
    char prompt[INPUT_LENGTH];
    char buffer[INPUT_LENGTH];
    int index = ask_for_transaction(list);

    if (index == -1) {
        return;
    }

    // Changes go into a copy first, so a mistake halfway through leaves the record unchanged
    updated = list->items[index];
    printf("Press Enter to keep the current value. The ID cannot be changed.\n");

    snprintf(prompt, sizeof(prompt), "Product name [%s]: ", updated.product);
    read_line(prompt, buffer, INPUT_LENGTH);
    if (buffer[0] != '\0') {
        if (!is_valid_text(buffer, PRODUCT_LENGTH)) {
            return;
        }
        strcpy(updated.product, buffer);
    }

    snprintf(prompt, sizeof(prompt), "Quantity sold [%d]: ", updated.quantity);
    read_line(prompt, buffer, INPUT_LENGTH);
    if (buffer[0] != '\0' && (!parse_int(buffer, &updated.quantity) || !is_valid_quantity(updated.quantity))) {
        printf("Error: the quantity must be a whole number from 1 to %d.\n", MAX_QUANTITY);
        return;
    }

    snprintf(prompt, sizeof(prompt), "Unit price [%.2f]: ", updated.unit_price);
    read_line(prompt, buffer, INPUT_LENGTH);
    if (buffer[0] != '\0' && (!parse_float(buffer, &updated.unit_price) || !is_valid_price(updated.unit_price))) {
        printf("Error: the unit price must be a number above 0 and up to %.0f.\n", MAX_PRICE);
        return;
    }

    snprintf(prompt, sizeof(prompt), "Transaction date [%s]: ", updated.date);
    read_line(prompt, buffer, INPUT_LENGTH);
    if (buffer[0] != '\0') {
        if (!is_valid_date(buffer)) {
            printf("Error: the date must be a real date written as YYYY-MM-DD.\n");
            return;
        }
        strcpy(updated.date, buffer);
    }

    updated.total = updated.quantity * updated.unit_price;
    list->items[index] = updated;
    printf("Transaction %d updated. New total amount: %.2f\n", updated.id, updated.total);
}

// Prints the main menu
void show_menu(void)
{
    printf("\n===== SALES TRANSACTION MANAGEMENT SYSTEM =====\n");
    printf(" 1. Add New Transaction\n");
    printf(" 2. Display All Transactions\n");
    printf(" 3. Update Transaction\n");
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
            case 1:
                add_transaction(&list);
                break;
            case 2:
                display_all_transactions(&list);
                break;
            case 3:
                update_transaction(&list);
                break;
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
