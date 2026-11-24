#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdio.h>
#include "sales.h"

int save_transactions(const struct TransactionList *list, const char *filename);
int read_file_line(FILE *file, char *line, int *line_number);
int parse_transaction_line(const char *line, struct Transaction *transaction);
int load_transactions(struct TransactionList *list, const char *filename);
void load_menu(struct TransactionList *list);

#endif
