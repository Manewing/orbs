#ifndef LIST_HH
#define LIST_HH

#include <stdlib.h>

typedef struct node {
  struct node* next;
  void*        data;
} node_t;
#define NODE_INIT { NULL, NULL }

typedef struct {
  node_t*   head;
  size_t    size;
} list_t;
#define LIST_INIT { NULL, 0 }

/// Adds element to list
/// @param[in] l    - The list to add the element to
/// @param[in] data - The element to add
void  list_add(list_t* l, void* data);

/// Removes an element from the list, given the element. Note that this
/// does not free the element.
/// @param[in] l    - The list to remove the element from
/// @param[in] data - The element to remove fromt the list
int   list_remove(list_t* l, void* data);

/// Removes all elements from the given list
/// @param[in] l         - The list to remove all elements from
/// @param[in] free_data - Function for freeing the elements in the list
void  list_remove_all(list_t* l, void (*free_data)(void*));

/// Prints the given list to stdout, in the style:
///
///   list[size] (list address):
///     node addr (data addr) -> next node addr
///     ...
///
/// @param[in] l - The list to print to stdout
void  list_print(list_t const *l);

#endif // #ifndef LIST_HH
