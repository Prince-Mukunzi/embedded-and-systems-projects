#include <stdio.h>
#include <errno.h>

#include "sales.h"
#include "input.h"
#include "utils.h"
#include "transaction.h"
#include "file_io.h"

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
