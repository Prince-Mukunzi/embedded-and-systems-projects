#include <stdio.h>

#include "sales.h"
#include "input.h"
#include "utils.h"
#include "transaction.h"
#include "filter.h"

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
        if (!read_double("Show transactions with a total above: ", &criteria.amount) || criteria.amount < 0) {
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
