#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "sales.h"

double calculate_total_revenue(const struct TransactionList *list);
double calculate_average_value(const struct TransactionList *list);
int find_highest_index(const struct TransactionList *list);
int find_lowest_index(const struct TransactionList *list);
int calculate_total_units(const struct TransactionList *list);
void show_total_revenue(const struct TransactionList *list);
void show_average_value(const struct TransactionList *list);
void show_highest_transaction(const struct TransactionList *list);
void show_lowest_transaction(const struct TransactionList *list);
void show_total_units(const struct TransactionList *list);
void process_sales_data(const struct TransactionList *list);

#endif
