#include "map.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "orb.h"
#include "config.h"

#define reset_str   "\n\33[0;0f"
#define rand_food   (rand() % global_config.food_rate == 1)

map_t* create_map(void) {

  // create new map
  map_t* map = (map_t*)malloc(sizeof(map_t));
  memset(map, 0, sizeof(map_t));

  // reset map
  return reset_map(map);
}

map_t* reset_map(map_t* map) {

  // free all orbs in map
  list_remove_all(&map->orbs, free_orb);

  // initialize buffers
  memset(map->buffer[0], ' ', W*H);
  map->buffer[0][W*H] = 0;
  memset(map->buffer[1], ' ', W*H);
  map->buffer[1][W*H] = 0;
  memset(map->data, ' ', W*H);
  map->data[W*H] = 0;

  return map;
}

void map_add_orb(map_t* map, orb_t* orb) {
  list_add(&map->orbs, orb);
}

void map_remove_orb(map_t* map, orb_t* orb) {
  list_remove(&map->orbs, orb);
}

void map_update_orb(map_t* map, orb_t* orb) {

  // let orb eat food from map
  char food = map->data[pos(orb->x, orb->y)];
  if (food != ' ') {
    orb_feed(orb, food);
    map->data[pos(orb->x, orb->y)] = ' ';
  }

  // draw orb to buffer
  map->buffer[map->buffer_idx][pos(orb->x, orb->y)] = orb->body;
}

void map_spawn_food(map_t* map) {

  // spawn food
  if (rand_food) {

    // get random position
    int fx = rand() % W;
    int fy = rand() % H;

    if (map->data[pos(fx, fy)] == global_config.food_types[0])
      map->data[pos(fx, fy)] = global_config.food_types[1];
    else
      map->data[pos(fx, fy)] = global_config.food_types[0];
  }

}

void update_map(map_t* map) {

  // update iteration
  map->iteration++;

  // add new food to map
  map_spawn_food(map);

  // switch buffer
  map->buffer_idx ^= 0x1;

  // copy data to buffer
  memcpy(map->buffer[map->buffer_idx], map->data, W*H);

  // update orbs
  node_t* node = map->orbs.head;
  while (node) {
    node_t* next = node->next;

    // orb might die
    orb_live(node->data, map);

    node = next;
  }


  // crossover
  // reset orb map
  orb_t* tmp_orb_map[W*H];
  memset(&tmp_orb_map, 0, W*H*sizeof(orb_t*));

  node = map->orbs.head;
  while (node) {
    orb_t* orb = node->data;


    if (orb->score > global_config.orb_scores[2]) {
      if (tmp_orb_map[pos(orb->x, orb->y)] == NULL) {
        tmp_orb_map[pos(orb->x, orb->y)] = orb;
      } else {
        orb_t* other = tmp_orb_map[pos(orb->x, orb->y)];

        // crossover orbs and mutate new
        orb_t* new_orb1 = orb_crossover(orb, other);
        orb_mutate(new_orb1);
        orb_t* new_orb2 = orb_crossover(orb, other);
        orb_mutate(new_orb2);

        // set position of new orbs and add to map
        new_orb1->x = wrap(orb->x+1, W, 0);
        new_orb1->y = orb->y;
        new_orb2->x = wrap(orb->x-1, W, 0);
        new_orb2->y = orb->y;
        map_add_orb(map, new_orb1);
        map_add_orb(map, new_orb2);

        // update parent scores
        orb->score >>= 2;
        other->score >>= 2;

        tmp_orb_map[pos(orb->x, orb->y)] = NULL;
      }
    }


    node = node->next;
  }
}

void draw_map(map_t* map) {
  // update map info
  int n = sprintf(map->buffer[map->buffer_idx],
      "[ORBS][0x%x] %d Hz, # %6ld: Orbs: %ld, FR: %d, MR: %d",
      global_config.seed, global_config.herz, map->iteration, map->orbs.size,
      global_config.food_rate, global_config.orb_mutation);
  map->buffer[map->buffer_idx][n] = ' ';

  printf(reset_str);
  printf("%s", map->buffer[map->buffer_idx]);
}

void free_map(void* map) {
  map_t* __map = map;
  list_remove_all(&__map->orbs, free_orb);
  free(map);
}
