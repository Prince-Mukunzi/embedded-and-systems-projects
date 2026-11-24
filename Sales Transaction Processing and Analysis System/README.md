# Sales Transaction Processing and Analysis System

A menu-driven C program for recording and analysing sales transactions. You can add, display, update and delete transactions, run calculations through an array of function pointers, filter and sort with callback functions, search, and print a sales report. Transactions live in a dynamic array that grows with `realloc`, and they are saved to a text file so they are still there next time.

## How to build and run

```bash
cd "Sales Transaction Processing and Analysis System"
make
./sales
```

To start with the sample data instead of an empty list:

```bash
cp sample_transactions.txt transactions.txt
./sales
```

`make clean` removes the compiled program. Without `make`, the same build is:

```bash
gcc -Wall -Wextra -std=c11 -o sales main.c input.c utils.c transaction.c operations.c filter.c search_sort.c report.c file_io.c
```

## Menu

```
 1. Add New Transaction
 2. Display All Transactions
 3. Update Transaction
 4. Delete Transaction
 5. Process Sales Data
 6. Filter Transactions (Callbacks)
 7. Search Transactions
 8. Sort Transactions
 9. Generate Sales Report
10. Save Transactions
11. Load Transactions
 0. Exit
```

## File structure

| File | What it contains |
|------|------------------|
| `sales.h` | All `#define` constants and the `Transaction`, `TransactionList` and `FilterCriteria` structs |
| `main.c` | The menu and the main loop that calls the right function for each option |
| `input.c` / `input.h` | Reading keyboard input safely (whole numbers, decimals, text, yes/no) |
| `utils.c` / `utils.h` | Case-insensitive text comparison, and checks for dates, quantities and prices |
| `transaction.c` / `transaction.h` | The dynamic list (create, grow, free) and add, display, update, delete |
| `operations.c` / `operations.h` | The five calculations and the function pointer array that runs them (option 5) |
| `filter.c` / `filter.h` | Callback condition functions and the filter that uses them (option 6) |
| `search_sort.c` / `search_sort.h` | Linear search (option 7), compare callbacks and bubble sort (option 8) |
| `report.c` / `report.h` | The sales report (option 9) |
| `file_io.c` / `file_io.h` | Saving to and loading from `transactions.txt` (options 10 and 11, and on startup) |
| `Makefile` | Builds the program with `make` |
| `sample_transactions.txt` | Ten sample transactions |
| `sample_report.txt` | The sales report and the five calculations run on the sample data |

## Structures

```c
struct Transaction {
    int id;
    char product[PRODUCT_LENGTH];
    int quantity;
    double unit_price;
    double total;
    char date[DATE_LENGTH];
};

struct TransactionList {
    struct Transaction *items;
    int count;
    int capacity;
};

struct FilterCriteria {
    double amount;
    char product[PRODUCT_LENGTH];
};
```

- `TransactionList` is the dynamic array: `items` points to memory for `capacity` transactions, and `count` says how many are actually used.
- `total` is always `quantity * unit_price`. It is calculated by the program when a transaction is added or updated, never typed in.
- Money is stored as `double`, and a price may have at most 2 decimals, so totals stay exact to the cent.
- `FilterCriteria` holds what the user wants to filter by. It is passed to every filter callback so they all have the same parameters.

## Pointers

- Every function that only reads the list takes `const struct TransactionList *list` and reads `list->items[i]`, `list->count`.
- Functions that change the list take `struct TransactionList *list` (for example `add_transaction`, `delete_transaction`, `sort_transactions`).
- `print_transaction(const struct Transaction *transaction)` receives the address of one transaction instead of a copy.
- The `read_int`, `read_double` and `parse_*` functions give back the value through a pointer parameter and use the return value to say whether the input was valid.
- In `load_transactions`, `*list = loaded;` copies the newly loaded list into the caller's list.

## Dynamic memory

| Where | What happens |
|-------|--------------|
| `create_list` (transaction.c) | `calloc` room for `INITIAL_CAPACITY` (4) transactions at startup |
| `grow_list` (transaction.c) | When the list is full, `realloc` it to double the capacity |
| `add_transaction` (transaction.c) | Calls `grow_list` before adding if `count == capacity` |
| `load_transactions` (file_io.c) | Builds a separate list with `create_list` and `grow_list`; frees the old list only after the new one loaded correctly |
| `free_list` (transaction.c) | `free` the items and set the pointer to `NULL`. Called before the program exits |

`realloc` always goes into a temporary pointer (`resized`) first. If it returns `NULL`, the old list is still valid, the program prints "not enough memory" and nothing is lost.

## Function pointers (option 5)

In `process_sales_data` (operations.c):

```c
void (*operations[OPERATION_COUNT])(const struct TransactionList *) = {
    show_total_revenue,
    show_average_value,
    show_highest_transaction,
    show_lowest_transaction,
    show_total_units
};
...
selected_operation = operations[option - 1];
selected_operation(list);
```

- `operations` is an array of 5 function pointers. All five functions take the same parameter and return nothing, so they fit in one array.
- The user's choice is used as the index, and the function is called through the pointer. There is no `switch` for these five operations.
- Each `show_*` function uses a calculation function: `calculate_total_revenue`, `calculate_average_value`, `find_highest_index`, `find_lowest_index`, `calculate_total_units`.

## Callbacks

**Filtering (option 6, filter.c)**

```c
void filter_transactions(const struct TransactionList *list,
                         int (*condition)(const struct Transaction *, const struct FilterCriteria *),
                         const struct FilterCriteria *criteria);
```

- `filter_transactions` does not know what it is filtering for. It calls `condition` on every transaction and prints the ones where it returns 1.
- The condition functions are `is_above_amount` (total above an amount) and `is_matching_product` (same product name, ignoring case).
- `count_matching` takes the same callback and counts the matches. `filter_menu` passes the chosen condition to both.
- `is_matching_product` is also reused by the sales report to add up the revenue for one product.

**Sorting (option 8, search_sort.c)**

```c
void sort_transactions(struct TransactionList *list,
                       int (*compare)(const struct Transaction *, const struct Transaction *));
```

- The compare callback returns a negative number, 0 or a positive number, like `strcmp`.
- Compare functions: `compare_by_id`, `compare_by_product`, `compare_by_total_ascending`, `compare_by_total_descending`.
- `compare_by_total_descending` simply calls `compare_by_total_ascending(b, a)` with the two transactions swapped.

## Searching and sorting algorithms

**Linear search** (`find_transaction_index`, `search_by_id`, `search_by_product`, `search_by_amount`): look at every transaction from the first to the last and compare it.

- By ID: exact match.
- By product: the text can appear anywhere in the product name, ignoring case (`contains_ignore_case`).
- By amount: totals within half a cent of the amount typed, because decimal numbers are stored with tiny rounding errors.

**Bubble sort** (`sort_transactions`): compare each pair of neighbours with the callback and swap them if it returns a positive number. After each pass the largest item has moved to the end. If a pass makes no swaps the list is sorted and the loop stops early. `qsort` is not used anywhere.

## File handling

Transactions are saved to `transactions.txt` as plain text, with `|` between fields:

```
10                                       <- number of transactions
201|Laptop|3|850.00|2550.00|2026-10-01   <- id|product|quantity|unit_price|total|date
...
```

- **Save** (`save_transactions`): option 10, and automatically when you choose 0. `|` is not allowed in a product name, so it can never break the file. Write errors are checked with `ferror` before `fclose`.
- **Load** (`load_transactions`): runs when the program starts, and with option 11. It reads the count, then each line with `sscanf` and `parse_transaction_line`, which checks every value again (positive ID, quantity, price, real date, and that the total equals quantity times price).
- A missing file on the first run is normal: the program starts with an empty list.
- A damaged file (bad count, missing line, invalid value, duplicate ID) gives an error with the line number. At startup the program stops, because saving an empty list on exit would overwrite the real data. With option 11 the list in memory is kept.
- Option 11 asks for confirmation first if there are transactions in memory, because unsaved changes would be lost.

## Error handling and validation

| Situation | What happens |
|-----------|--------------|
| Menu choice that is empty, text, or not on the menu | Error message, menu shows again |
| Sub-menu choice out of range (options 5, 6, 7, 8) | Error message, back to the main menu |
| Transaction ID that is not a positive whole number | Rejected |
| Transaction ID that already exists | Rejected ("already exists") |
| Empty or blank product name, or longer than 49 characters | Rejected |
| Product name containing `|` | Rejected (it is the file separator) |
| Quantity of 0, negative, text, or above 1,000,000 | Rejected |
| Price of 0, negative, text, `nan`, `inf`, above 1,000,000, or more than 2 decimals | Rejected |
| Date not in `YYYY-MM-DD` form, or not a real date (for example `2026-02-30`, or 29 February in a non-leap year) | Rejected |
| Update, delete or search for an ID that does not exist | "no transaction has ID ..." |
| Update where one field is invalid | Nothing is changed (edits go to a copy first) |
| Delete without typing `y` | Cancelled |
| Any option that needs data while the list is empty | "No transactions have been recorded yet." |
| Average with zero transactions | Never divides: `calculate_average_value` returns 0 when the count is 0 |
| Empty search text, or a negative filter amount | Error message |
| `calloc` or `realloc` returns `NULL` | Error message, existing data kept |
| Data file missing | Starts with an empty list |
| Data file exists but cannot be opened | Error message; the file is not touched |
| Data file is damaged | Error with the line number; the file is not touched |
| Data file cannot be written | Error message. On exit it asks whether to exit without saving |
| Keyboard input ends (Ctrl+D) | Saves and exits cleanly |

## Testing

- Every function was tested right after it was written (happy path and the bad inputs in the table above) and committed only after it worked.
- A scripted session covering every menu option and the invalid cases was run with `-fsanitize=address,undefined` on Linux (gcc, with leak detection) and macOS (clang): no memory errors, no undefined behaviour, no leaks. The output was identical on both.
- Loading was tested with hand-made files: a bad count, a negative count, a missing line, a wrong total, a zero quantity, an impossible date, a price with 3 decimals, a duplicate ID, an unreadable file, an empty list, and a file with more transactions than the starting capacity.
- After splitting `main.c` into modules, the same session produced exactly the same output and the same saved file as the single-file version.
- The code compiles with zero warnings using `-Wall -Wextra` on gcc (including `-O2`) and clang.

## Sample output

```
========== SALES REPORT ==========
Total number of transactions : 10
Total revenue                : 6089.29
Average transaction value    : 608.93
Highest transaction value    : 2550.00 (ID 201, Laptop)
Lowest transaction value     : 135.00 (ID 210, Keyboard)
Total quantity sold          : 97
==================================
Enter a product name to see its revenue: Laptop
Revenue for selected product : 3400.00 (Laptop, 2 transaction(s))
```

The full output is in `sample_report.txt`.
