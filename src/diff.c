#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include <bin.h>

void diff_push(diff_t **head, diff_t *diff) {
    assert(head != NULL && diff != NULL);

    if (*head == NULL) {
        *head = diff;

        diff->next = diff;
        diff->prev = diff;

        return;
    }

    diff->next = *head;
    diff->prev = (*head)->prev;

    (*head)->prev->next = diff;
    (*head)->prev = diff;
}

diff_t *diff_pop(diff_t **head) {
    diff_t *pop, *newhead;

    assert(head != NULL && *head != NULL);

    pop = *head;
    if (pop->next == pop || pop->prev == pop) {
        *head = NULL;

        return pop;
    }

    newhead = pop->next;
    (*head)->prev->next = newhead;
    newhead->prev = (*head)->prev;

    *head = newhead;
    return pop;
}

void diff_free(diff_t *head) {
    diff_t *pop, **headptr;

    if (head == NULL) {
        return;
    }

    headptr = &head;
    while (*headptr != NULL) {
        pop = diff_pop(headptr);

        free(pop);
    }
}

void print_diff(diff_t **head) {
    diff_t *diff;

    assert(head != NULL && *head != NULL);

    do {
        diff = diff_pop(head);

        printf("offset 0x%llX\n", diff->offset);
        printf("length %zu\n", diff->length);
        
        if (diff->oldbuf != NULL) {
            printf("-");
            for (size_t i = 0; i < diff->length; i++) {
                printf(" 0x%hhX", diff->oldbuf[i]);
            }
            printf("\n");

            free(diff->oldbuf);
        }

        if (diff->newbuf != NULL) {
            printf("+");
            for (size_t i = 0; i < diff->length; i++) {
                printf(" 0x%hhX", diff->newbuf[i]);
            }
            printf("\n");

            free(diff->newbuf);
        }

        printf("\n");

        free(diff);
    } while (*head != NULL);
}
