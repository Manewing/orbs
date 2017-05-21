#include "config.h"

config_t global_config = {
  .orb_count          =     40,
  .orb_score          =   1000,
  .orb_mutation       =  10000,
  .orb_bodies         = {  '.',  'o',  'O' },
  .orb_scores         = { 2000, 4000, 8000 },
  .food_rate          =     20,
  .food_types         = {  '+',  '#' },
  .food_scores        = { 1000, 4000 }
};
