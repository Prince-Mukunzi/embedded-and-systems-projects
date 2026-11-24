#ifndef REPORT_H
#define REPORT_H

#include "sales.h"

void print_product_revenue(const struct TransactionList *list);
void generate_sales_report(const struct TransactionList *list);

#endif
