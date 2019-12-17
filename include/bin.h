#ifndef BIN_H
#define BIN_H

#include <stdio.h>
#include <stdint.h>

#define ERR(...) fprintf(stderr, __VA_ARGS__)

struct input {
    FILE *file;
    off_t offset;
    uint8_t value;
};
typedef struct input input_t;

enum operation {
    READ,
    WRITE
};
typedef enum operation operation_t;

input_t *parse_arguments(int argc, char **argv, operation_t op);
void free_input(input_t *in);

off_t file_size(FILE *file);

int read_offset(FILE *file, off_t offset, uint8_t *value);
int write_offset(FILE *file, off_t offset, uint8_t value);

struct diff {
    off_t offset;
    size_t length;

    uint8_t *oldbuf;
    uint8_t *newbuf;

    struct diff *next;
    struct diff *prev;
};
typedef struct diff diff_t;

void diff_push(diff_t **head, diff_t *diff);
diff_t *diff_pop(diff_t **head);
void diff_free(diff_t *head);

void print_diff(diff_t **head);

#endif
