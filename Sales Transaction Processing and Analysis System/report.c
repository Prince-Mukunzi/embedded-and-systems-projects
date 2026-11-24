#include <stdio.h>

#include "sales.h"
#include "input.h"
#include "operations.h"
#include "filter.h"
#include "report.h"

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
