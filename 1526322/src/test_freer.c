#include <stdio.h>
#include <stdlib.h>

#include "freer.h"

struct test_struct {
    int n;
    int p;
};

int main(int argc, char ** argv) {
    struct free_list *freelist = calloc(1, sizeof(struct free_list));

    char *one = calloc(35, sizeof(char));
    free_list_add(one, freelist);
 
    char *two = calloc(420, sizeof(char));
    free_list_add(two, freelist);

    struct test_struct *three = calloc(1, sizeof(struct test_struct));
    free_list_add(three, freelist);

    free_list_free_all(freelist);
  
    return 0;
}
