#ifndef LIST_H_
#define LIST_H_
#include <stdlib.h>

typedef struct ListNode_t {
  void *data;
  struct ListNode_t *next;
  struct ListNode_t *previous;
} ListNode_t;

typedef struct List_t {
  size_t size;
  ListNode_t *head;
  ListNode_t *tail;
} List_t;

// Creates a fresh list
void list_create(void **out);
// Returns the size of the list
size_t list_size(List_t *list);
// Destroys list
void list_destroy(List_t *list);

// Creates a fresh node and returns a pointer to the data
void list_node_insert(List_t *list, void *data, void **out);

// Assigns the head node to out
void list_get_head(List_t *list, void **out);
// Assigns the tail node to out
void list_get_tail(List_t *list, void **out);

// Removes head from list
void list_remove_head(List_t *list);
// Removes tail from list
void list_remove_tail(List_t *list);

void list_foreach(List_t *list, void (*operation)(void *e));

/* void list_get_next(List_t *list, void **curr, void **out); */
/* void list_get_previous(List_t *list, void **curr, void **out); */

#endif // LIST_H_
