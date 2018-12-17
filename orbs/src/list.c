#include "list.h"

#include <stdio.h>

#include "mm.h"

static struct mm *list_mm = NULL;

void list_add(list_t *l, void *data) {

  if (list_mm == NULL)
    list_mm = create_mm(sizeof(node_t));

  node_t *node = (node_t *)mm_elem_create(list_mm);
  node->data = data;
  node->next = l->head;
  l->head = node;
  l->size++;
}

int list_remove(list_t *l, void *data) {
  for (node_t *node = l->head, **pp = &l->head; node;
       pp = &node->next, node = node->next) {

    if (node->data == data) {
      *pp = node->next;
      mm_elem_free(list_mm, node);
      l->size--;
      return 1;
    }
  }
  return 0;
}

void list_remove_all(list_t *l, void (*free_data)(void *)) {
  node_t *node = l->head;

  while (node) {
    node_t *next = node->next;

    free_data(node->data);
    mm_elem_free(list_mm, node);

    node = next;
  }

  l->head = NULL;
  l->size = 0;
}

void list_print(list_t const *l) {
  printf("list[%ld] (%p):\n", l->size, l);
  for (node_t *node = l->head; node; node = node->next) {
    printf("  %p (%p) -> %p\n", node, node->data, node->next);
  }
}
