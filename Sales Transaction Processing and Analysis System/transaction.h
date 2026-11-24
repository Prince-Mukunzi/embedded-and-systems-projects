#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "sales.h"

int create_list(struct TransactionList *list);
void free_list(struct TransactionList *list);
int grow_list(struct TransactionList *list);
int find_transaction_index(const struct TransactionList *list, int id);
void add_transaction(struct TransactionList *list);
void print_table_header(void);
void print_transaction(const struct Transaction *transaction);
void display_all_transactions(const struct TransactionList *list);
int ask_for_transaction(const struct TransactionList *list);
void update_transaction(struct TransactionList *list);
void delete_transaction(struct TransactionList *list);

#endif
