#include <stdio.h>

#include "sales.h"
#include "input.h"
#include "transaction.h"
#include "operations.h"
#include "filter.h"
#include "search_sort.h"
#include "report.h"
#include "file_io.h"

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
