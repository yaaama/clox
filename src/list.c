#include "include/list.h"
#include "include/util.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define IS_NOT_NULL(a) (a != NULL)
/*****************************************************************************/
/*                               Dynamic array                               */
/*****************************************************************************/

array_T *array_create(size_t item_size) {

  array_T *array = calloc(1, sizeof(array_T));

  array->size = 0;
  array->item_size = item_size;

  return array;
}

void array_push(array_T *array, void *item) {

  assert(array != NULL && "Array was null");

  if (!item) {
    return;
  }

  array->size += 1;

  if (!array->items) {
    array->items = calloc(1, array->item_size);
  } else {
    array->items =
        realloc(array->items, (array->size * sizeof(array->item_size)));
  }

  array->items[array->size - 1] = item;
}

// TODO Implement this
void array_destroy(array_T *array);

/*****************************************************************************/
/*                             Doubly linked list                            */
/*****************************************************************************/

/**
 *  \brief Creates a new doubly linked list and assigns it to the out argument.
 *
 *  This function creates a new list, allocating memory for it using malloc.
 *  Size is initialised as 0 and both head and tail will point to NULL.
 *
 *  \param out Where a reference to the list will be passed to.
 *  \return void
 */
void list_create(void **out) {

  List_t *list = calloc(1, sizeof(List_t));
  list->size = 0;
  list->head = NULL;

  list->tail = NULL;

  printf("List created!");

  *out = list;
}

/**
 *  \brief Returns the size of a given list.
 *
 *  It will access the list structs "size" member and return it.
 *
 *  \param list pointer.
 *  \return size_t size.
 */
size_t list_size(List_t *list) {

  assert(list != NULL && "List is NULL.");
  return list->size;
}

/**
 *  \brief Destroys a list and frees all of its memory.
 *
 *  Detailed description
 *
 *  \param param
 *  \return return type
 */
void list_destroy(List_t *list) {

  assert(list != NULL);

  ListNode_t *curr = list->head;

  // If head is null or the size is 0 then just free the List and return
  if (curr == NULL || list->size == 0) {
    free(list);
    printf("The list was empty! Freeing only the list itself.\n");
    return;
  }
  // Next node
  ListNode_t *next = list->head->next;

  size_t counter = 0;
  // Free up all of the nodes
  while (curr) {
    free(curr->data);
    free(curr);
    counter++;
    curr = next;
    if (!curr->next) {
      break;
    }
    next = curr->next;
  }
  // Free the tail
  free(list->tail->data);
  free(list->tail);

  counter++;
  fprintf(stdout, "%s: Freed %zu items.", __FUNCTION__, counter);
  // Finally free the list
  free(list);
}

void list_node_insert(List_t *list, void *data, void **out) {

  assert(list != NULL);
  assert(data != NULL);

  ListNode_t *toInsert = malloc(sizeof(ListNode_t));

  toInsert->data = data;
  toInsert->next = NULL;
  toInsert->previous = NULL;

  // If the list is empty currently
  if (list->head == NULL) {
    list->head = toInsert;
    list->tail = toInsert;
  } else {
    /* ListNode_t *currTail = list->tail; */
    toInsert->previous = list->tail;
    list->tail->next = NULL;
    list->tail->next = toInsert;
    list->tail = toInsert;
  }

  list->size++;
  printf("New node added! New size: %zu\n", list->size);

  if (out == NULL) {
    return;
  }
  *out = list->tail->data;
}

void list_remove_head(List_t *list) {
  assert(list != NULL);

  ListNode_t *newHead;

  if (IS_NOT_NULL(list->head)) {
    newHead = list->head->next;
    newHead->previous = NULL;
    free(list->head);
    list->size--;
    printf("Head was removed. New size: %zu\n", list->size);
    return;

  } else {
    printf("Head was null!\n");
    return;
  }
}

void list_remove_tail(List_t *list) {
  assert(list != NULL);

  ListNode_t *newTail;

  if (IS_NOT_NULL(list->tail)) {
    newTail = list->tail->next;
    newTail->previous = NULL;
    free(list->tail);
    printf("Tail was removed. New size: %zu\n", list->size);
    list->size--;

  } else {
    printf("tail was null!\n");
  }
}

void list_get_head(List_t *list, void **out) {

  if (list == NULL || list->size == 0) {
    printf("List is empty (or NULL)\n");
    return;
  }

  *out = list->head->data;
}

void list_get_tail(List_t *list, void **out) {

  if (list == NULL || list->size == 0) {
    printf("List is empty (or NULL)\n");
    return;
  }

  *out = list->tail->data;
}

void list_foreach(List_t *list, void (*operation)(void *e)) {

  size_t count = 0;

  ListNode_t *curr = list->head;

  while (curr) {
    operation(curr->data);
    curr = curr->next;
    count++;
  }

  printf("list_foreach: %zu number of operations performed.\n", count);
}
