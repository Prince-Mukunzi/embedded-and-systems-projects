#ifndef SALES_H
#define SALES_H

#define DATA_FILE "transactions.txt"
#define PRODUCT_LENGTH 50
#define DATE_LENGTH 11
#define INPUT_LENGTH 100
#define INITIAL_CAPACITY 4
#define MAX_QUANTITY 1000000
#define MAX_PRICE 1000000.0
#define MIN_YEAR 2000
#define MAX_YEAR 2100
#define OPERATION_COUNT 5
#define FILE_LINE_LENGTH 256

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

#endif
