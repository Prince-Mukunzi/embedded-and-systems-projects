#include <stdio.h>

#include "sales.h"
#include "input.h"
#include "utils.h"
#include "transaction.h"
#include "search_sort.h"

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
    double amount;
    int found = 0;
    int i;

    if (!read_double("Enter the transaction amount: ", &amount) || amount <= 0) {
        printf("Error: the amount must be a number above 0.\n");
        return;
    }

    for (i = 0; i < list->count; i++) {
        // Decimal numbers are stored with tiny rounding errors, so "equal" means within half a cent
        if (is_close(list->items[i].total, amount, 0.005)) {
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
