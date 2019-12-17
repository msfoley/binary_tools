#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <bin.h>

//#define BUFSZ 4096 // One page
#define BUFSZ 1048576 // One megabyte

int main(int argc, char **argv) {
    FILE *oldfile, *newfile;
    off_t oldsz, newsz, smallestsz, offset;
    size_t readsz;
    int ret = 1;
    uint8_t *oldbuf, *newbuf;
    diff_t *head = NULL;

    if (argc != 3) {
        ERR("Incorrect arguments, see: %s <file1> <file2>\n", argv[0]);

        return 1;
    }

    oldfile = fopen(argv[1], "rb");
    if (oldfile == NULL) {
        ERR("Failed to open %s\n", argv[1]);

        return 1;
    }
    
    newfile = fopen(argv[2], "rb");
    if (oldfile == NULL) {
        ERR("Failed to open %s\n", argv[1]);

        fclose(oldfile);
        return 1;
    }

    oldbuf = malloc(sizeof(*oldbuf) * BUFSZ);
    if (oldbuf == NULL) {
        ERR("Failed to allocate memory\n");

        fclose(oldfile);
        fclose(newfile);
        return 1;
    }

    newbuf = malloc(sizeof(*newbuf) * BUFSZ);
    if (newbuf == NULL) {
        ERR("Failed to allocate memory\n");

        fclose(oldfile);
        fclose(newfile);
        free(oldbuf);
        return 1;
    }

    oldsz = file_size(oldfile);
    if (oldsz < 0) {
        ERR("Failed to get size of %s: %s\n", argv[1], strerror(errno));

        goto failure;
    }
    
    newsz = file_size(newfile);
    if (newsz < 0) {
        ERR("Failed to get size of %s: %s\n", argv[2], strerror(errno));

        goto failure;
    }

    offset = 0;
    smallestsz = newsz < oldsz ? newsz : oldsz;

    while (offset < smallestsz) {
        size_t readlen = 0;
        diff_t *diff = NULL;
        int result;

        if (offset + BUFSZ > smallestsz) {
            readsz = smallestsz - offset;
        } else {
            readsz = BUFSZ;
        }

        readlen = fread(newbuf, sizeof(*newbuf), readsz, newfile);
        if (readlen < readsz) {
            result = ferror(newfile);
            if (result != 0) {
                ERR("Failed to read %s: %s\n", argv[2], strerror(errno));

                ret = result;
                goto failure;
            }

            result = feof(newfile);
            if (result != 0) {
                ERR("Found EOF in %s (should never happen)\n", argv[2]);

                ret = result;
                goto failure;
            }
        }

        readlen = fread(oldbuf, sizeof(*oldbuf), readsz, oldfile);
        if (readlen < readsz) {
            result = ferror(oldfile);
            if (result != 0) {
                ERR("Failed to read %s: %s\n", argv[1], strerror(errno));

                ret = result;
                goto failure;
            }

            result = feof(oldfile);
            if (result != 0) {
                ERR("Found EOF in %s (should never happen)\n", argv[1]);

                ret = result;
                goto failure;
            }
        }
        
        for (size_t i = 0; i < readsz; i++) {
            if (newbuf[i] == oldbuf[i]) {
                if (diff != NULL) {
                    diff->oldbuf = malloc(sizeof(*diff->oldbuf) * diff->length);
                    if (diff->oldbuf == NULL) {
                        ERR("Failed to allocate memory\n");

                        free(diff);
                        goto failure;
                    }

                    diff->newbuf = malloc(sizeof(*diff->oldbuf) * diff->length);
                    if (diff->newbuf == NULL) {
                        ERR("Failed to allocate memory\n");

                        free(diff->oldbuf);
                        free(diff);
                        goto failure;
                    }

                    memcpy(diff->oldbuf, oldbuf + diff->offset, diff->length);
                    memcpy(diff->newbuf, newbuf + diff->offset, diff->length);

                    diff_push(&head, diff);
                    diff = NULL;
                }

                continue;
            }

            if (diff == NULL) {
                diff = malloc(sizeof(*diff));
                if (diff == NULL) {
                    ERR("Failed to allocate memory\n");

                    goto failure;
                }

                diff->offset = offset + i;
                diff->length = 1;
            } else {
                diff->length++;
            }
        }

        offset += readsz;
    }

    if (oldsz != newsz) {
        size_t readlen;
        int result;

        diff_t *diff = malloc(sizeof(*diff));
        if (diff == NULL) {
            ERR("Failed to allocate memory\n");

            goto failure;
        }

        if (oldsz > newsz) {
            diff->offset = newsz;
            diff->length = oldsz - newsz;
            diff->newbuf = NULL;
            diff->oldbuf = malloc(sizeof(*diff->oldbuf) * diff->length);
            if (diff->oldbuf == NULL) {
                ERR("Failed to allocate memory\n");

                goto failure;
            }
            
            readlen = fread(diff->oldbuf, sizeof(*diff->oldbuf), diff->length, oldfile);
            if (readlen < readsz) {
                result = ferror(oldfile);
                if (result != 0) {
                    ERR("Failed to read %s: %s\n", argv[1], strerror(errno));

                    ret = result;
                    goto failure;
                }

                result = feof(oldfile);
                if (result != 0) {
                    ERR("Found EOF in %s (should never happen)\n", argv[1]);

                    ret = result;
                    goto failure;
                }
            }
        } else {
            diff->offset = oldsz;
            diff->length = newsz - oldsz;
            diff->oldbuf = NULL;
            diff->newbuf = malloc(sizeof(*diff->newbuf) * diff->length);
            if (diff->newbuf == NULL) {
                ERR("Failed to allocate memory\n");

                goto failure;
            }
        
            readlen = fread(diff->newbuf, sizeof(*diff->newbuf), diff->length, newfile);
            if (readlen < readsz) {
                result = ferror(newfile);
                if (result != 0) {
                    ERR("Failed to read %s: %s\n", argv[2], strerror(errno));

                    ret = result;
                    goto failure;
                }

                result = feof(newfile);
                if (result != 0) {
                    ERR("Found EOF in %s (should never happen)\n", argv[2]);

                    ret = result;
                    goto failure;
                }
            }
        }

        diff_push(&head, diff);
    }

    print_diff(&head);
    
    ret = 0;
failure:
    diff_free(head);
    fclose(oldfile);
    fclose(newfile);
    free(oldbuf);
    free(newbuf);

    return ret;
}
