#include <stdlib.h>

#include <bin.h>

int main(int argc, char **argv) {
    input_t *args;
    uint8_t value;
    int result;

    args = parse_arguments(argc, argv, READ);

    result = read_offset(args->file, args->offset, &value);
    if (result != 0) {
        fprintf(stderr, "Failed to read offset.\n");

        exit(1);
    }

    printf("Value: 0x%X (%c)\n", value, (char) value);

    free_input(args);
}
