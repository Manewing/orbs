#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "map.h"
#include "orb.h"
#include "config.h"

#define Hz(x) (1000000 / (x))

int main() {
  srand(0);

  map_t* map = create_map();

  int l = 0;
  for (; l < global_config.orb_count; l++) {
    orb_t* orb = create_orb();
    reset_orb_genes(orb);
    map_add_orb(map, orb);
  }

  while(1) {
    update_map(map);
    draw_map(map);
    usleep(Hz(200));
  }

  free_map(map);
  return 0;
}
