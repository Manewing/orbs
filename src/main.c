#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "map.h"
#include "orb.h"

#define Hz(x) (1000000 / (x))

int main() {
  srand(0);

  map_t* m = create_map();

  int l = 0;
  for (; l < 40; l++)
    map_add_orb(m, create_orb());

  while(1) {
    update_map(m);
    draw_map(m);
    usleep(Hz(100));
  }

  free_map(m);
  return 0;
}
