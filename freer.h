struct free_list {
    void *elem;
    struct free_list *next;
};

struct free_list *free_list_create();
int free_list_add(void *elem, struct free_list *list);
int free_list_free_all(struct free_list *list);
