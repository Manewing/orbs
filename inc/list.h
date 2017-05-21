#ifndef LIST_HH
#define LIST_HH

#include <stdlib.h>

typedef struct node {
  struct node* next;
  void*        data;
} node_t;

typedef struct {
  node_t*   head;
  size_t    size;
} list_t;

void  list_add(list_t* l, void* data);
int   list_remove(list_t* l, void* data);
void  list_remove_all(list_t* l, void (*free_data)(void*));
void  list_print(list_t* l);

#endif // #ifndef LIST_HH
