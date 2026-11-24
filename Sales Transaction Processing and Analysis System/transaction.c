#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sales.h"
#include "input.h"
#include "utils.h"
#include "transaction.h"

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
    if (!read_double("Unit price: ", &new_transaction.unit_price) || !is_valid_price(new_transaction.unit_price)) {
        printf("Error: the unit price must be above 0, up to %.0f, with at most 2 decimals.\n", MAX_PRICE);
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
    if (buffer[0] != '\0' && (!parse_double(buffer, &updated.unit_price) || !is_valid_price(updated.unit_price))) {
        printf("Error: the unit price must be above 0, up to %.0f, with at most 2 decimals.\n", MAX_PRICE);
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
