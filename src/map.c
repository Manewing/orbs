#include "map.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "orb.h"

map_t* create_map(void) {
  map_t* map = (map_t*)malloc(sizeof(map_t));
  return reset_map(map);
}

map_t* reset_map(map_t* map) {
  memset(map->buffer, ' ', W*H);
  map->buffer[W*H] = 0;
  memset(map->data, ' ', W*H);
  map->data[W*H] = 0;
  return map;
}

void update_map(map_t* map) {
  // spawn food
  int fx = rand() % W;
  int fy = rand() % H;
  if (map->data[pos(fx, fy)] == '+')
    map->data[pos(fx, fy)] = '#';
  else
    map->data[pos(fx, fy)] = '+';

  // copy data to buffer
  memcpy(map->buffer, map->data, W*H);

  // update orbs
  node_t* node = map->orbs.head;
  while (node) {
    node_t* next = node->next;

    // orb might die
    orb_live(node->data, map);

    node = next;
  }
}

void map_add_orb(map_t* map, orb_t* orb) {
  list_add(&map->orbs, orb);
}

void map_remove_orb(map_t* map, orb_t* orb) {
  list_remove(&map->orbs, orb);
}

void map_update_orb(map_t* map, orb_t* orb) {
  // let orb eat food from map
  char at = map->data[pos(orb->x, orb->y)];
  if (at == '+')
    orb->score += 100;
  else if (at == '#')
    orb->score += 400;
  map->data[pos(orb->x, orb->y)] = ' ';

  // draw orb to buffer
  map->buffer[pos(orb->x, orb->y)] = orb->o;
}

#define reset_str "\n\33[0;0f"

void draw_map(map_t* map) {
  printf(reset_str);
  printf("%s\n", map->buffer);
}

void free_map(map_t* map) {
  free(map);
}
