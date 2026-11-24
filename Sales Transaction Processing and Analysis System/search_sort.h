#ifndef SEARCH_SORT_H
#define SEARCH_SORT_H

#include "sales.h"

void search_by_id(const struct TransactionList *list);
void search_by_product(const struct TransactionList *list);
void search_by_amount(const struct TransactionList *list);
void search_transactions(const struct TransactionList *list);
int compare_by_id(const struct Transaction *a, const struct Transaction *b);
int compare_by_product(const struct Transaction *a, const struct Transaction *b);
int compare_by_total_ascending(const struct Transaction *a, const struct Transaction *b);
int compare_by_total_descending(const struct Transaction *a, const struct Transaction *b);
void sort_transactions(struct TransactionList *list,
                       int (*compare)(const struct Transaction *, const struct Transaction *));
void sort_menu(struct TransactionList *list);

#endif
