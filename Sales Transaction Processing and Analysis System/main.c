#include <stdio.h>
#include <errno.h>

#include "sales.h"
#include "input.h"
#include "utils.h"
#include "transaction.h"
#include "operations.h"
#include "filter.h"

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

// Prints the revenue and number of sales for the product the user types
void print_product_revenue(const struct TransactionList *list)
{
    struct FilterCriteria criteria;
    double revenue = 0.0;
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

// Writes every transaction to a text file. Returns 1 on success.
int save_transactions(const struct TransactionList *list, const char *filename)
{
    FILE *file;
    const struct Transaction *transaction;
    int write_failed;
    int i;

    file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: could not open %s for writing.\n", filename);
        return 0;
    }

    // Line 1 is the number of transactions, then one line per transaction
    fprintf(file, "%d\n", list->count);
    for (i = 0; i < list->count; i++) {
        transaction = &list->items[i];
        fprintf(file, "%d|%s|%d|%.2f|%.2f|%s\n", transaction->id, transaction->product, transaction->quantity,
                transaction->unit_price, transaction->total, transaction->date);
    }

    // fclose also writes out anything still waiting in memory, so its result must be checked
    write_failed = ferror(file);
    if (fclose(file) != 0 || write_failed) {
        printf("Error: something went wrong while writing %s.\n", filename);
        return 0;
    }

    printf("%d transaction(s) saved to %s.\n", list->count, filename);
    return 1;
}

// Reads the next line of the file and counts it. Returns 0 when the file has ended.
int read_file_line(FILE *file, char *line, int *line_number)
{
    if (fgets(line, FILE_LINE_LENGTH, file) == NULL) {
        return 0;
    }
    (*line_number)++;
    return 1;
}

// Fills a transaction from one saved line. Returns 1 only if every value is present and valid.
int parse_transaction_line(const char *line, struct Transaction *transaction)
{
    int fields = sscanf(line, "%d|%49[^|]|%d|%lf|%lf|%10[^\n]", &transaction->id, transaction->product,
                        &transaction->quantity, &transaction->unit_price, &transaction->total, transaction->date);

    if (fields != 6) {
        return 0;
    }

    // The saved total must match quantity times price, which catches a wrongly edited file
    return transaction->id > 0 && is_valid_quantity(transaction->quantity) && is_valid_price(transaction->unit_price)
           && is_valid_date(transaction->date)
           && is_close(transaction->total, transaction->quantity * transaction->unit_price, 0.005);
}

// Reads the file into a new list. The current list is replaced only if everything loads correctly.
int load_transactions(struct TransactionList *list, const char *filename)
{
    FILE *file;
    struct TransactionList loaded;
    struct Transaction transaction;
    char line[FILE_LINE_LENGTH];
    int line_number = 0;
    int total = 0;
    int ok = 1;
    int i;

    file = fopen(filename, "r");
    if (file == NULL) {
        if (errno == ENOENT) {
            printf("No saved transactions found in %s.\n", filename);
            return 1;
        }
        printf("Error: %s exists but could not be opened.\n", filename);
        return 0;
    }

    if (!create_list(&loaded)) {
        printf("Error: not enough memory to load the transactions.\n");
        fclose(file);
        return 0;
    }

    if (!read_file_line(file, line, &line_number) || sscanf(line, "%d", &total) != 1 || total < 0) {
        ok = 0;
    }

    for (i = 0; ok && i < total; i++) {
        if (!read_file_line(file, line, &line_number) || !parse_transaction_line(line, &transaction)) {
            ok = 0;
        } else if (find_transaction_index(&loaded, transaction.id) != -1) {
            printf("Error: transaction ID %d appears twice in the file.\n", transaction.id);
            ok = 0;
        } else if (loaded.count == loaded.capacity && !grow_list(&loaded)) {
            printf("Error: not enough memory to load the transactions.\n");
            ok = 0;
        } else {
            loaded.items[loaded.count] = transaction;
            loaded.count++;
        }
    }
    fclose(file);

    if (!ok) {
        printf("Error: %s could not be loaded (problem near line %d).\n", filename, line_number);
        free_list(&loaded);
        return 0;
    }

    free_list(list);
    *list = loaded;
    printf("%d transaction(s) loaded from %s.\n", list->count, filename);
    return 1;
}

// Menu option 11: reloads the saved file, after warning that it replaces what is in memory
void load_menu(struct TransactionList *list)
{
    if (list->count > 0
        && !ask_yes_no("Loading replaces the transactions in memory. Unsaved changes will be lost. Continue? (y/n): ")) {
        printf("Load cancelled.\n");
        return;
    }
    load_transactions(list, DATA_FILE);
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
    printf("10. Save Transactions\n");
    printf("11. Load Transactions\n");
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

    // Stopping here protects the file: saving an empty list on exit would overwrite it
    if (!load_transactions(&list, DATA_FILE)) {
        printf("Fix or move %s, then start the program again.\n", DATA_FILE);
        free_list(&list);
        return 1;
    }

    while (running) {
        show_menu();

        if (!read_line("Choose an option: ", input, INPUT_LENGTH)) {
            printf("\nInput ended. Saving before closing.\n");
            save_transactions(&list, DATA_FILE);
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
            case 10:
                save_transactions(&list, DATA_FILE);
                break;
            case 11:
                load_menu(&list);
                break;
            case 0:
                if (save_transactions(&list, DATA_FILE)
                    || ask_yes_no("Saving failed. Exit without saving? (y/n): ")) {
                    running = 0;
                }
                break;
            default:
                printf("Error: %d is not a menu option.\n", choice);
        }
    }

    printf("Goodbye.\n");
    free_list(&list);
    return 0;
}
