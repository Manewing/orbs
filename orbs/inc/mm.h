#ifndef MM_HH
#define MM_HH

#include <stdlib.h>

struct mm {
  void** buffer;
  size_t buffer_size;
  size_t buffer_idx;
  size_t elem_size;
};

struct mm* create_mm(const size_t elem_size);
void* mm_elem_create(struct mm* m);
void mm_elem_free(struct mm* m, void* elem);
void free_mm(struct mm* m);

#endif // #ifndef MM_HH
