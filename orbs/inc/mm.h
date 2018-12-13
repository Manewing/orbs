#ifndef MM_HH
#define MM_HH

#include <stdlib.h>

/// Memory Manager, basically a simple stack that allows reusage of heap
/// objects by pushing free objects on it and poping objects off it when
/// needed.
struct mm {
  /// Buffer for storing freed objects
  void** buffer;

  /// The current size/capacity of the buffer
  size_t buffer_size;

  /// The current index in the buffer
  size_t buffer_idx;

  /// The size of an element, needed for allocating new elements
  size_t elem_size;
};

/// Creates a new memory manager given the size of a single element
/// @param[in] elem_size - The size of a single element
/// @return The memory manager
struct mm* create_mm(const size_t elem_size);

/// Creates a new element or returns an old one from the stack, created
/// elements should be free with the memory manager free method.
/// @param[in] m - The memory manager to create an element with, get/from
/// @return The new element
void* mm_elem_create(struct mm* m);

/// Frees an element by pushing it onto the stack of the memory manager
/// @param[in] m    - The memory manager
/// @param[in] elem - The element to free
void mm_elem_free(struct mm* m, void* elem);

/// Frees the memory manager and all the elements that it holds
/// @param[in] m - The memory manager to destroy
void free_mm(struct mm* m);

#endif // #ifndef MM_HH
