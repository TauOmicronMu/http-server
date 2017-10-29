#include <stdio.h>
#include <stdlib.h>

#include "freer.h"

// TODO: consider a better approach using a tagged union and accepting 
//       either a pointer to a block of alloc'd memory, or a function
//       pointer that destroys a block of alloc'd memory.

struct free_list *free_list_create() {
    struct free_list *list = calloc(1, sizeof(struct free_list));
    return list;
}

int free_list_add(void *elem, struct free_list *list) {
    struct free_list *curr = list;
    while(curr) {
        if(!curr->next) {
            curr->next = free_list_create();
            curr->next->elem = elem;
            return 0;
        }
        curr = curr->next;
    }
    return -1;    
}

int free_list_free_all(struct free_list *list) {
    struct free_list *curr = list;
    while(curr) {
        struct free_list *next = curr->next;
        free(curr->elem);
        curr->elem = NULL;
        free(curr);
        curr = next;
    }
    return 0;
}
