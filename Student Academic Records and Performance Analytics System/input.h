#ifndef INPUT_H
#define INPUT_H

int read_line(const char *prompt, char *buffer, int size);
int parse_int(const char *text, int *value);
int read_int(const char *prompt, int *value);
int is_blank(const char *text);
int is_valid_text(const char *text, int size);
int read_text(const char *prompt, char *destination, int size);
int is_valid_year(int year);
int ask_yes_no(const char *question);
int parse_float(const char *text, float *value);
int read_float(const char *prompt, float *value);
void to_lowercase(const char *source, char *destination, int size);
int compare_ignore_case(const char *first, const char *second);
int same_text_ignore_case(const char *first, const char *second);
int is_valid_max_marks(float max_marks);
int is_valid_marks(float marks, float max_marks);
int contains_ignore_case(const char *text, const char *part);

#endif
