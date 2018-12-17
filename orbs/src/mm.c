#include "mm.h"

#define MM_INIT_SIZE 256

struct mm *create_mm(const size_t elem_size) {
  struct mm *m = (struct mm *)malloc(sizeof(struct mm));

  // initialize new mm
  m->elem_size = elem_size;
  m->buffer_idx = 0;
  m->buffer_size = MM_INIT_SIZE;
  m->buffer = (void **)malloc(sizeof(void *) * m->buffer_size);

  return m;
}

void *mm_elem_create(struct mm *m) {
  // have element in buffer?
  if (m->buffer_idx != 0)
    return m->buffer[--m->buffer_idx];

  // no element in buffer create new
  return malloc(m->elem_size);
}

void mm_elem_free(struct mm *m, void *elem) {

  // check if enough space in buffer left
  if (m->buffer_idx >= m->buffer_size) {
    // run out of buffer space, realloc
    m->buffer_size *= 2;
    m->buffer = realloc(m->buffer, sizeof(void *) * m->buffer_size);
  }

  // store element
  m->buffer[m->buffer_idx++] = elem;
}

void free_mm(struct mm *m) {
  size_t l;

  // free elements
  for (l = 0; l < m->buffer_idx; l++)
    free(m->buffer[l]);

  // free buffer
  free(m->buffer);

  // free mm
  free(m);
}
