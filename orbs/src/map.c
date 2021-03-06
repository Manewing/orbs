#include "map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "orb.h"

#define reset_str "\n\33[0;0f"

map_config_t map_config = {.food_rate = 30, .food_scores = {1000, 4000}};

map_t *create_map(void) {

  // create new map
  map_t *map = (map_t *)malloc(sizeof(map_t));
  memset(map, 0, sizeof(map_t));

  // create statistics
  if (strlen(global_config.stats) != 0) {
    map->stats = create_stats(global_config.stats);
  }

  // set food rate
  map->food_rate = map_config.food_rate;
  map->food_inc_rate = 100000;

  // reset map
  return reset_map(map);
}

map_t *reset_map(map_t *map) {

  // free all orbs in map
  list_remove_all(&map->orbs, free_orb);

  // initialize buffers
  memset(map->buffer[0], ' ', W * H);
  map->buffer[0][W * H] = 0;
  memset(map->buffer[1], ' ', W * H);
  map->buffer[1][W * H] = 0;
  memset(map->data, ' ', W * H);
  map->data[W * H] = 0;

  return map;
}

void map_add_orb(map_t *map, orb_t *orb) { list_add(&map->orbs, orb); }

void map_remove_orb(map_t *map, orb_t *orb) { list_remove(&map->orbs, orb); }

void map_update_orb(map_t *map, orb_t *orb) {

  // let orb eat food from map
  char food = map->data[pos(orb->x, orb->y)];
  if (food != ' ') {
    orb_feed(orb, food);
    map->data[pos(orb->x, orb->y)] = ' ';
  }

  // draw orb to buffer
  map_buffer(map, 0)[pos(orb->x, orb->y)] = orb->body;
}

void map_spawn_food(map_t *map) {
  // check if food rate needs to be increased
  if (map->food_inc_timer >= map->food_inc_rate) {
    map->food_rate++;
    map->food_inc_rate += 2000;
    map->food_inc_timer = 0;
  }
  map->food_inc_timer++;

  // spawn food
  if ((rand() % map->food_rate) == 1) {

    // get random position
    int fx = rand() % W;
    int fy = rand() % H;

    if (map->data[pos(fx, fy)] == '+') {
      map->data[pos(fx, fy)] = '#';
    } else {
      map->data[pos(fx, fy)] = '+';
    }
  }
}

void update_map(map_t *map) {

  // update iteration
  map->iteration++;

  // update statistics
  if (map->stats != NULL) {
    update_stats(map->stats, map);
  }

  // add new food to map
  map_spawn_food(map);

  // switch buffer
  map->buffer_idx ^= 0x1;

  // copy data to buffer
  memcpy(map_buffer(map, 0), map->data, W * H);

  // update orbs
  node_t *node = map->orbs.head;
  while (node) {
    node_t *next = node->next;

    // orb might die
    orb_live(node->data, map);

    node = next;
  }

  // crossover
  // reset orb map
  orb_t *tmp_orb_map[W * H];
  memset(&tmp_orb_map, 0, W * H * sizeof(orb_t *));

  node = map->orbs.head;
  while (node) {
    orb_t *orb = node->data;

    // TODO make this check a function of the orb
    if (orb->score > orb_config.scores[2]) {
      if (tmp_orb_map[pos(orb->x, orb->y)] == NULL) {
        tmp_orb_map[pos(orb->x, orb->y)] = orb;
      } else {
        orb_t *other = tmp_orb_map[pos(orb->x, orb->y)];

        // crossover orbs and mutate new
        orb_t *new_orb1 = orb_crossover(orb, other);
        orb_mutate(new_orb1);
        orb_t *new_orb2 = orb_crossover(orb, other);
        orb_mutate(new_orb2);

        // set position of new orbs and add to map
        new_orb1->x = wrap(orb->x + 1, W, 0);
        new_orb1->y = orb->y;
        new_orb2->x = wrap(orb->x - 1, W, 0);
        new_orb2->y = orb->y;
        map_add_orb(map, new_orb1);
        map_add_orb(map, new_orb2);

        // update parent scores
        orb->score >>= 2;
        other->score >>= 2;
        new_orb1->score = orb->score;
        new_orb2->score = other->score;

        tmp_orb_map[pos(orb->x, orb->y)] = NULL;
      }
    }

    node = node->next;
  }
}

void draw_map(map_t *map) {
  // update map info
  int n = sprintf(map_buffer(map, 0),
                  "[ORBS][0x%x] %d Hz, # %6ld: Orbs: %ld, "
                  "FoodRate: 1/%d, MutationRate: 1/%d",
                  global_config.seed, global_config.herz, map->iteration,
                  map->orbs.size, map->food_rate, orb_config.mutation);
  map_buffer(map, 0)[n] = ' ';

  printf(reset_str);
  printf("%s", map_buffer(map, 0));
}

void free_map(void *map) {
  map_t *__map = map;

  // free orbs
  list_remove_all(&__map->orbs, free_orb);

  // free statistics
  if (__map->stats != NULL) {
    free_stats(__map->stats);
  }

  free(map);
}
