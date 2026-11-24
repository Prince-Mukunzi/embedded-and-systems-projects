#include <stdio.h>

#include "sales.h"
#include "input.h"
#include "transaction.h"
#include "operations.h"

// Adds up the total amount of every transaction
double calculate_total_revenue(const struct TransactionList *list)
{
    double revenue = 0.0;
    int i;

    for (i = 0; i < list->count; i++) {
        revenue += list->items[i].total;
    }
    return revenue;
}

// Average amount per transaction, or 0 when there are no transactions
double calculate_average_value(const struct TransactionList *list)
{
    // Dividing by a count of zero is not allowed
    if (list->count == 0) {
        return 0.0;
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
