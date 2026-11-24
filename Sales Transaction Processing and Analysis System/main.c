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
#define OPERATION_COUNT 5

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

struct FilterCriteria {
    float amount;
    char product[PRODUCT_LENGTH];
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

// Copies text into destination with every letter in lowercase
void to_lowercase(const char *source, char *destination, int size)
{
    int i;

    for (i = 0; source[i] != '\0' && i < size - 1; i++) {
        destination[i] = tolower((unsigned char) source[i]);
    }
    destination[i] = '\0';
}

// Like strcmp but ignores case: negative if first comes before second, 0 if equal, positive if after
int compare_ignore_case(const char *first, const char *second)
{
    char first_lower[INPUT_LENGTH];
    char second_lower[INPUT_LENGTH];

    to_lowercase(first, first_lower, INPUT_LENGTH);
    to_lowercase(second, second_lower, INPUT_LENGTH);
    return strcmp(first_lower, second_lower);
}

// Returns 1 if two texts are the same when upper and lower case are ignored
int same_text_ignore_case(const char *first, const char *second)
{
    return compare_ignore_case(first, second) == 0;
}

// Returns 1 if part appears anywhere inside text, ignoring upper and lower case
int contains_ignore_case(const char *text, const char *part)
{
    char text_lower[INPUT_LENGTH];
    char part_lower[INPUT_LENGTH];

    to_lowercase(text, text_lower, INPUT_LENGTH);
    to_lowercase(part, part_lower, INPUT_LENGTH);
    return strstr(text_lower, part_lower) != NULL;
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

// Asks a yes/no question. Returns 1 only if the user types y or Y.
int ask_yes_no(const char *question)
{
    char buffer[INPUT_LENGTH];

    read_line(question, buffer, INPUT_LENGTH);
    return strcmp(buffer, "y") == 0 || strcmp(buffer, "Y") == 0;
}

// Menu option 4: removes a transaction and closes the gap it leaves in the list
void delete_transaction(struct TransactionList *list)
{
    int deleted_id;
    int i;
    int index = ask_for_transaction(list);

    if (index == -1) {
        return;
    }

    print_table_header();
    print_transaction(&list->items[index]);
    if (!ask_yes_no("Are you sure you want to delete this transaction? (y/n): ")) {
        printf("Delete cancelled.\n");
        return;
    }

    deleted_id = list->items[index].id;
    for (i = index; i < list->count - 1; i++) {
        list->items[i] = list->items[i + 1];
    }
    list->count--;

    printf("Transaction %d deleted.\n", deleted_id);
}

// Adds up the total amount of every transaction
float calculate_total_revenue(const struct TransactionList *list)
{
    float revenue = 0.0f;
    int i;

    for (i = 0; i < list->count; i++) {
        revenue += list->items[i].total;
    }
    return revenue;
}

// Average amount per transaction, or 0 when there are no transactions
float calculate_average_value(const struct TransactionList *list)
{
    // Dividing by a count of zero is not allowed
    if (list->count == 0) {
        return 0.0f;
    }
    return calculate_total_revenue(list) / list->count;
}

// Returns the position of the transaction with the largest total, or -1 if the list is empty
int find_highest_index(const struct TransactionList *list)
{
    int highest = -1;
    int i;

    for (i = 0; i < list->count; i++) {
        if (highest == -1 || list->items[i].total > list->items[highest].total) {
            highest = i;
        }
    }
    return highest;
}

// Returns the position of the transaction with the smallest total, or -1 if the list is empty
int find_lowest_index(const struct TransactionList *list)
{
    int lowest = -1;
    int i;

    for (i = 0; i < list->count; i++) {
        if (lowest == -1 || list->items[i].total < list->items[lowest].total) {
            lowest = i;
        }
    }
    return lowest;
}

// Adds up the quantity of every transaction
int calculate_total_units(const struct TransactionList *list)
{
    int units = 0;
    int i;

    for (i = 0; i < list->count; i++) {
        units += list->items[i].quantity;
    }
    return units;
}

// Sales operation 1
void show_total_revenue(const struct TransactionList *list)
{
    printf("Total revenue: %.2f\n", calculate_total_revenue(list));
}

// Sales operation 2
void show_average_value(const struct TransactionList *list)
{
    printf("Average transaction value: %.2f\n", calculate_average_value(list));
}

// Sales operation 3
void show_highest_transaction(const struct TransactionList *list)
{
    printf("Highest-value transaction:\n");
    print_table_header();
    print_transaction(&list->items[find_highest_index(list)]);
}

// Sales operation 4
void show_lowest_transaction(const struct TransactionList *list)
{
    printf("Lowest-value transaction:\n");
    print_table_header();
    print_transaction(&list->items[find_lowest_index(list)]);
}

// Sales operation 5
void show_total_units(const struct TransactionList *list)
{
    printf("Total units sold: %d\n", calculate_total_units(list));
}

// Menu option 5: the user picks an operation and it is called through a function pointer
void process_sales_data(const struct TransactionList *list)
{
    // Every operation takes the same parameter and returns nothing, so they fit in one array
    void (*operations[OPERATION_COUNT])(const struct TransactionList *) = {
        show_total_revenue,
        show_average_value,
        show_highest_transaction,
        show_lowest_transaction,
        show_total_units
    };
    const char *operation_names[OPERATION_COUNT] = {
        "Calculate total revenue",
        "Calculate average transaction value",
        "Identify the highest-value transaction",
        "Identify the lowest-value transaction",
        "Calculate total units sold"
    };
    void (*selected_operation)(const struct TransactionList *);
    int option;
    int i;

    // The highest and lowest operations need at least one transaction to point to
    if (list->count == 0) {
        printf("No transactions have been recorded yet.\n");
        return;
    }

    printf("\n----- PROCESS SALES DATA -----\n");
    for (i = 0; i < OPERATION_COUNT; i++) {
        printf("%d. %s\n", i + 1, operation_names[i]);
    }
    if (!read_int("Choose an operation: ", &option) || option < 1 || option > OPERATION_COUNT) {
        printf("Error: please choose a number from 1 to %d.\n", OPERATION_COUNT);
        return;
    }

    selected_operation = operations[option - 1];
    selected_operation(list);
}

// Callback condition: the transaction total is above the chosen amount
int is_above_amount(const struct Transaction *transaction, const struct FilterCriteria *criteria)
{
    return transaction->total > criteria->amount;
}

// Callback condition: the transaction is for the chosen product (case is ignored)
int is_matching_product(const struct Transaction *transaction, const struct FilterCriteria *criteria)
{
    return same_text_ignore_case(transaction->product, criteria->product);
}

// Prints every transaction for which the condition callback returns 1
void filter_transactions(const struct TransactionList *list,
                         int (*condition)(const struct Transaction *, const struct FilterCriteria *),
                         const struct FilterCriteria *criteria)
{
    int printed_header = 0;
    int i;

    for (i = 0; i < list->count; i++) {
        if (condition(&list->items[i], criteria)) {
            if (!printed_header) {
                print_table_header();
                printed_header = 1;
            }
            print_transaction(&list->items[i]);
        }
    }
}

// Counts the transactions for which the condition callback returns 1
int count_matching(const struct TransactionList *list,
                   int (*condition)(const struct Transaction *, const struct FilterCriteria *),
                   const struct FilterCriteria *criteria)
{
    int count = 0;
    int i;

    for (i = 0; i < list->count; i++) {
        if (condition(&list->items[i], criteria)) {
            count++;
        }
    }
    return count;
}

// Menu option 6: the chosen condition function is passed as a callback
void filter_menu(const struct TransactionList *list)
{
    struct FilterCriteria criteria;
    int (*condition)(const struct Transaction *, const struct FilterCriteria *);
    int matches;
    int option;

    if (list->count == 0) {
        printf("No transactions have been recorded yet.\n");
        return;
    }

    printf("\n----- FILTER TRANSACTIONS -----\n");
    printf("1. Transactions above a sales amount\n");
    printf("2. Transactions for a product\n");
    if (!read_int("Choose a filter: ", &option) || option < 1 || option > 2) {
        printf("Error: please choose 1 or 2.\n");
        return;
    }

    if (option == 1) {
        if (!read_float("Show transactions with a total above: ", &criteria.amount) || criteria.amount < 0) {
            printf("Error: the amount must be a number of 0 or more.\n");
            return;
        }
        condition = is_above_amount;
    } else {
        if (!read_text("Product name: ", criteria.product, PRODUCT_LENGTH)) {
            return;
        }
        condition = is_matching_product;
    }

    filter_transactions(list, condition, &criteria);
    matches = count_matching(list, condition, &criteria);
    if (matches == 0) {
        printf("No transactions match this filter.\n");
    } else {
        printf("%d of %d transaction(s) match this filter.\n", matches, list->count);
    }
}

// Prints the transaction with the ID the user types
void search_by_id(const struct TransactionList *list)
{
    int index = ask_for_transaction(list);

    if (index != -1) {
        print_table_header();
        print_transaction(&list->items[index]);
    }
}

// Linear search: prints every transaction whose product name contains the text typed
void search_by_product(const struct TransactionList *list)
{
    char search_text[INPUT_LENGTH];
    int found = 0;
    int i;

    read_line("Enter the product name (or part of it): ", search_text, INPUT_LENGTH);
    if (is_blank(search_text)) {
        printf("Error: the search text cannot be empty.\n");
        return;
    }

    for (i = 0; i < list->count; i++) {
        if (contains_ignore_case(list->items[i].product, search_text)) {
            if (found == 0) {
                print_table_header();
            }
            print_transaction(&list->items[i]);
            found++;
        }
    }

    if (found == 0) {
        printf("No transactions match '%s'.\n", search_text);
    } else {
        printf("%d transaction(s) found.\n", found);
    }
}

// Linear search: prints every transaction whose total amount equals the amount typed
void search_by_amount(const struct TransactionList *list)
{
    float amount;
    int found = 0;
    int i;

    if (!read_float("Enter the transaction amount: ", &amount) || amount <= 0) {
        printf("Error: the amount must be a number above 0.\n");
        return;
    }

    for (i = 0; i < list->count; i++) {
        // Decimal numbers are stored with tiny rounding errors, so "equal" means within half a cent
        if (fabsf(list->items[i].total - amount) < 0.005f) {
            if (found == 0) {
                print_table_header();
            }
            print_transaction(&list->items[i]);
            found++;
        }
    }

    if (found == 0) {
        printf("No transactions have a total of %.2f.\n", amount);
    } else {
        printf("%d transaction(s) found.\n", found);
    }
}

// Menu option 7
void search_transactions(const struct TransactionList *list)
{
    int option;

    if (list->count == 0) {
        printf("No transactions have been recorded yet.\n");
        return;
    }

    printf("\n----- SEARCH TRANSACTIONS -----\n");
    printf("1. By transaction ID\n");
    printf("2. By product name\n");
    printf("3. By transaction amount\n");
    if (!read_int("Choose an option: ", &option)) {
        option = 0;
    }

    switch (option) {
        case 1:
            search_by_id(list);
            break;
        case 2:
            search_by_product(list);
            break;
        case 3:
            search_by_amount(list);
            break;
        default:
            printf("Error: please choose 1, 2 or 3.\n");
    }
}

// Compare callback: smaller ID first. Negative means a comes first, positive means b comes first.
int compare_by_id(const struct Transaction *a, const struct Transaction *b)
{
    if (a->id < b->id) {
        return -1;
    }
    if (a->id > b->id) {
        return 1;
    }
    return 0;
}

// Compare callback: product names from A to Z, ignoring case
int compare_by_product(const struct Transaction *a, const struct Transaction *b)
{
    return compare_ignore_case(a->product, b->product);
}

// Compare callback: smaller total amount first
int compare_by_total_ascending(const struct Transaction *a, const struct Transaction *b)
{
    if (a->total < b->total) {
        return -1;
    }
    if (a->total > b->total) {
        return 1;
    }
    return 0;
}

// Compare callback: larger total amount first (the ascending order with a and b swapped)
int compare_by_total_descending(const struct Transaction *a, const struct Transaction *b)
{
    return compare_by_total_ascending(b, a);
}

// Bubble sort: the compare callback decides whether two neighbours are in the wrong order
void sort_transactions(struct TransactionList *list,
                       int (*compare)(const struct Transaction *, const struct Transaction *))
{
    struct Transaction temp;
    int swapped;
    int i;
    int j;

    for (i = 0; i < list->count - 1; i++) {
        swapped = 0;

        for (j = 0; j < list->count - 1 - i; j++) {
            if (compare(&list->items[j], &list->items[j + 1]) > 0) {
                temp = list->items[j];
                list->items[j] = list->items[j + 1];
                list->items[j + 1] = temp;
                swapped = 1;
            }
        }

        if (!swapped) {
            break;
        }
    }
}

// Menu option 8
void sort_menu(struct TransactionList *list)
{
    int option;

    if (list->count == 0) {
        printf("No transactions have been recorded yet.\n");
        return;
    }

    printf("\n----- SORT TRANSACTIONS -----\n");
    printf("1. By transaction ID\n");
    printf("2. By product name (A to Z)\n");
    printf("3. By total amount (lowest first)\n");
    printf("4. By total amount (highest first)\n");
    if (!read_int("Choose an option: ", &option)) {
        option = 0;
    }

    switch (option) {
        case 1:
            sort_transactions(list, compare_by_id);
            break;
        case 2:
            sort_transactions(list, compare_by_product);
            break;
        case 3:
            sort_transactions(list, compare_by_total_ascending);
            break;
        case 4:
            sort_transactions(list, compare_by_total_descending);
            break;
        default:
            printf("Error: please choose a number from 1 to 4.\n");
            return;
    }

    printf("Transactions sorted.\n");
    display_all_transactions(list);
}

// Prints the revenue and number of sales for the product the user types
void print_product_revenue(const struct TransactionList *list)
{
    struct FilterCriteria criteria;
    float revenue = 0.0f;
    int sales = 0;
    int i;

    if (!read_text("Enter a product name to see its revenue: ", criteria.product, PRODUCT_LENGTH)) {
        return;
    }

    for (i = 0; i < list->count; i++) {
        if (is_matching_product(&list->items[i], &criteria)) {
            revenue += list->items[i].total;
            sales++;
        }
    }

    if (sales == 0) {
        printf("No transactions found for '%s'.\n", criteria.product);
    } else {
        printf("Revenue for selected product : %.2f (%s, %d transaction(s))\n", revenue, criteria.product, sales);
    }
}

// Menu option 9
void generate_sales_report(const struct TransactionList *list)
{
    const struct Transaction *highest;
    const struct Transaction *lowest;

    if (list->count == 0) {
        printf("No transactions have been recorded yet, so there is nothing to report.\n");
        return;
    }

    highest = &list->items[find_highest_index(list)];
    lowest = &list->items[find_lowest_index(list)];

    printf("\n========== SALES REPORT ==========\n");
    printf("Total number of transactions : %d\n", list->count);
    printf("Total revenue                : %.2f\n", calculate_total_revenue(list));
    printf("Average transaction value    : %.2f\n", calculate_average_value(list));
    printf("Highest transaction value    : %.2f (ID %d, %s)\n", highest->total, highest->id, highest->product);
    printf("Lowest transaction value     : %.2f (ID %d, %s)\n", lowest->total, lowest->id, lowest->product);
    printf("Total quantity sold          : %d\n", calculate_total_units(list));
    printf("==================================\n");

    print_product_revenue(list);
}

// Prints the main menu
void show_menu(void)
{
    printf("\n===== SALES TRANSACTION MANAGEMENT SYSTEM =====\n");
    printf(" 1. Add New Transaction\n");
    printf(" 2. Display All Transactions\n");
    printf(" 3. Update Transaction\n");
    printf(" 4. Delete Transaction\n");
    printf(" 5. Process Sales Data\n");
    printf(" 6. Filter Transactions (Callbacks)\n");
    printf(" 7. Search Transactions\n");
    printf(" 8. Sort Transactions\n");
    printf(" 9. Generate Sales Report\n");
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
            case 4:
                delete_transaction(&list);
                break;
            case 5:
                process_sales_data(&list);
                break;
            case 6:
                filter_menu(&list);
                break;
            case 7:
                search_transactions(&list);
                break;
            case 8:
                sort_menu(&list);
                break;
            case 9:
                generate_sales_report(&list);
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
