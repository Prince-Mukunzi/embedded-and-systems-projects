#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "sales.h"
#include "utils.h"

// Copies text into destination with every letter in lowercase
void to_lowercase(const char *source, char *destination, int size)
{
    int i;

    for (i = 0; source[i] != '\0' && i < size - 1; i++) {
        destination[i] = tolower((unsigned char) source[i]);
    }
    destination[i] = '\0';
}

// Like strcmp but ignores case: negative if first comes before second, 0 if equal, positive if after
int compare_ignore_case(const char *first, const char *second)
{
    char first_lower[INPUT_LENGTH];
    char second_lower[INPUT_LENGTH];

    to_lowercase(first, first_lower, INPUT_LENGTH);
    to_lowercase(second, second_lower, INPUT_LENGTH);
    return strcmp(first_lower, second_lower);
}

// Returns 1 if two texts are the same when upper and lower case are ignored
int same_text_ignore_case(const char *first, const char *second)
{
    return compare_ignore_case(first, second) == 0;
}

// Returns 1 if part appears anywhere inside text, ignoring upper and lower case
int contains_ignore_case(const char *text, const char *part)
{
    char text_lower[INPUT_LENGTH];
    char part_lower[INPUT_LENGTH];

    to_lowercase(text, text_lower, INPUT_LENGTH);
    to_lowercase(part, part_lower, INPUT_LENGTH);
    return strstr(text_lower, part_lower) != NULL;
}

// Returns 1 for leap years, when February has 29 days
int is_leap_year(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

// Returns 1 if the text is a real calendar date written as YYYY-MM-DD
int is_valid_date(const char *date)
{
    int days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int year;
    int month;
    int day;
    int i;

    if (strlen(date) != 10 || date[4] != '-' || date[7] != '-') {
        return 0;
    }
    for (i = 0; i < 10; i++) {
        if (i != 4 && i != 7 && !isdigit((unsigned char) date[i])) {
            return 0;
        }
    }

    sscanf(date, "%d-%d-%d", &year, &month, &day);
    if (year < MIN_YEAR || year > MAX_YEAR || month < 1 || month > 12) {
        return 0;
    }
    if (month == 2 && is_leap_year(year)) {
        days_in_month[1] = 29;
    }
    return day >= 1 && day <= days_in_month[month - 1];
}

// Returns 1 if the quantity is a positive whole number within the limit
int is_valid_quantity(int quantity)
{
    return quantity > 0 && quantity <= MAX_QUANTITY;
}

// Returns 1 when two decimal numbers differ by less than the tolerance
int is_close(double first, double second, double tolerance)
{
    double difference = first - second;

    if (difference < 0) {
        difference = -difference;
    }
    return difference < tolerance;
}

// Returns 1 if the unit price is above zero, within the limit, and has at most 2 decimal places
int is_valid_price(double price)
{
    double cents = price * 100;
    long whole_cents;

    if (price > 0 && price <= MAX_PRICE) {
        // A real price is a whole number of cents, so cents must be (almost exactly) a whole number
        whole_cents = (long)(cents + 0.5);
        return is_close(cents, whole_cents, 0.000001);
    }
    return 0;
}
