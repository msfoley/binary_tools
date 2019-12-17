#include <stdlib.h>

#include <bin.h>

int main(int argc, char **argv) {
    input_t *args;
    int result;

    args = parse_arguments(argc, argv, WRITE);

    result = write_offset(args->file, args->offset, args->value);
    if (result != 0) {
        fprintf(stderr, "Failed to write offset.\n");

        exit(1);
    }

    free_input(args);
}
