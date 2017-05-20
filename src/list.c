#include "list.h"

void list_add(list_t* l, void* data) {
  node_t* node = (node_t*)malloc(sizeof(node_t));

  node->data = data;
  node->next = NULL;

  if (l->head == NULL) {
    l->head = node;
    l->tail = node;
  } else {
    l->tail->next = node;
    l->tail = node;
  }

  l->size++;
}

int list_remove(list_t* l, void* data) {
  node_t** pp = &l->head;
  node_t* node = l->head;

  while (node) {

    if (node->data == data) {
      *pp = node->next;
      free(node);
      l->size--;
      return 1;
    }

    pp = &node->next;
    node = node->next;
  }

  return 0;
}
