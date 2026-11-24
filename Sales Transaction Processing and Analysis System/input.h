#ifndef INPUT_H
#define INPUT_H

int read_line(const char *prompt, char *buffer, int size);
int parse_int(const char *text, int *value);
int read_int(const char *prompt, int *value);
int parse_double(const char *text, double *value);
int read_double(const char *prompt, double *value);
int is_blank(const char *text);
int is_valid_text(const char *text, int size);
int read_text(const char *prompt, char *destination, int size);
int ask_yes_no(const char *question);

#endif
