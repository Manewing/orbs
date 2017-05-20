#ifndef LIST_HH
#define LIST_HH

#include <stdlib.h>

typedef struct node {
  struct node* next;
  void* data;
} node_t;

typedef struct {
  node_t* head;
  node_t* tail;
  size_t size;
} list_t;

void list_add(list_t* l, void* data);
int list_remove(list_t* l, void* data);

#endif // #ifndef LIST_HH
