#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

#include <bin.h>

input_t *parse_arguments(int argc, char **argv, operation_t op) {
    unsigned char value;
    long long int offset;
    off_t size;
    input_t *parsed_args;

    assert(argv != NULL);

    if (argc == 2 && (strcmp(argv[1], "-h") == 0)) {
        switch(op) {
            case READ:
                printf("%s <file> <binary offset>\n", argv[0]);
                break;
            case WRITE:
                printf("%s <file> <binary offset> <value>\n", argv[0]);
                break;
            default:
                exit(1);
                break;
        }

        exit(0);
    }

    if ((op == WRITE && argc != 4) || (op == READ && argc != 3)) {
        ERR("Invalid arguments, see \"%s -h\"\n", argv[0]);

        exit(1);
    }

    parsed_args = malloc(sizeof(*parsed_args));
    if (parsed_args == NULL) {
        ERR("Failed to allocate memory.\n");
    }

    switch (op) {
        case READ:
            parsed_args->file = fopen(argv[1], "rb");
            break;
        case WRITE:
            parsed_args->file = fopen(argv[1], "r+b");
            break;
        default:
            free(parsed_args);
            exit(1);

            break;
    }

    if (parsed_args->file == NULL) {
        ERR("Failed to open %s: %s\n", argv[1], strerror(errno));

        free(parsed_args);
        exit(1);
    }

    if (sscanf(argv[2], "%lli", &offset) != 1) {
        ERR("Invalid offset: %s\n", argv[2]);
        
        goto failure;
    }
    // TODO: better way for hex input of off_t
    // This is unsafe
    parsed_args->offset = (off_t) offset;

    if (op == WRITE) {
        if (sscanf(argv[3], "%hhi", &value) != 1) {
            ERR("Invalid binary value: %s\n", argv[3]);
            
            goto failure;
        }

        parsed_args->value = (uint8_t) value;
    }

    size = file_size(parsed_args->file);
    if (size == -1) {
        ERR("Failed to read %s: %s\n", argv[1], strerror(errno));
        
        goto failure;
    } else if (size == -2) {
        ERR("Failed to get size of %s: %s\n", argv[1], strerror(errno));

        goto failure;
    } 
    
    if (size < offset) {
        ERR("Offset %s is larger than the size of %s\n", argv[2], argv[1]);

        goto failure;
    }

    return parsed_args;

failure:
    fclose(parsed_args->file);
    free(parsed_args);
    exit(1);
}

off_t file_size(FILE *file) {
    off_t size;
    
    fseek(file, 0, SEEK_END);
    if (ferror(file)) {
        errno = ferror(file);

        return -1;
    }
    
    size = ftell(file);
    if (size < 0) {
        return -2;
    }
    
    rewind(file);

    return size;
}

void free_input(input_t *in) {
    assert(in != NULL);

    fclose(in->file);
    free(in);
}

int read_offset(FILE *file, off_t offset, uint8_t *value) {
    int result;

    assert(file != NULL && value != NULL);

    fseek(file, offset, SEEK_SET);
    fread(value, sizeof(*value), 1, file);
    
    result = ferror(file);
    if (result != 0) {
        ERR("Failed to read file: %s\n", strerror(result));
    }

    rewind(file);

    return 0;
}

int write_offset(FILE *file, off_t offset, uint8_t value) {
    int result;

    assert(file != NULL);

    printf("Value to write: 0x%X\n", value);

    result = fseek(file, offset, SEEK_SET);
    if (result != 0) {
        ERR("Failed to seek file: %s\n", strerror(result));
    }
    

    result = ferror(file);
    if (result != 0) {
        ERR("Failed to seek file: %s\n", strerror(result));
    }

    fwrite(&value, sizeof(value), 1, file);
    
    result = ferror(file);
    if (result != 0) {
        ERR("Failed to write file: %s\n", strerror(result));
    }

    rewind(file);

    return 0;
}


