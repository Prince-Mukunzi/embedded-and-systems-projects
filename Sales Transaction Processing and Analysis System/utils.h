#ifndef UTILS_H
#define UTILS_H

void to_lowercase(const char *source, char *destination, int size);
int compare_ignore_case(const char *first, const char *second);
int same_text_ignore_case(const char *first, const char *second);
int contains_ignore_case(const char *text, const char *part);
int is_leap_year(int year);
int is_valid_date(const char *date);
int is_valid_quantity(int quantity);
int is_close(double first, double second, double tolerance);
int is_valid_price(double price);

#endif
