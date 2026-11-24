#ifndef FILTER_H
#define FILTER_H

#include "sales.h"

int is_above_amount(const struct Transaction *transaction, const struct FilterCriteria *criteria);
int is_matching_product(const struct Transaction *transaction, const struct FilterCriteria *criteria);
void filter_transactions(const struct TransactionList *list,
                         int (*condition)(const struct Transaction *, const struct FilterCriteria *),
                         const struct FilterCriteria *criteria);
int count_matching(const struct TransactionList *list,
                   int (*condition)(const struct Transaction *, const struct FilterCriteria *),
                   const struct FilterCriteria *criteria);
void filter_menu(const struct TransactionList *list);

#endif
