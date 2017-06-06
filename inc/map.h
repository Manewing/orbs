#ifndef MAP_HH
#define MAP_HH

#include "list.h"

#define W 80
#define H 24

struct orb_t;

typedef struct map_t {
  size_t  iteration;
  int     buffer_idx;
  char    buffer[2][W*H + 1];
  char    data[W*H + 1];
  list_t  orbs;
} map_t;

map_t*    create_map(void);
map_t*    reset_map(map_t* map);

void      map_add_orb(map_t* map, struct orb_t* orb);
void      map_remove_orb(map_t* map, struct orb_t* orb);
void      map_update_orb(map_t* map, struct orb_t* orb);
void      map_spawn_food(map_t* map);

void      update_map(map_t* map);
void      draw_map(map_t* map);
void      free_map(void* map);

#define pos(x, y) ((x) + ((y)*W))
#define wrap(x, max, min) ((x) >= max ? min : ((x) < min ? max-1 : (x)))

#endif // #ifndef MAP_HH
