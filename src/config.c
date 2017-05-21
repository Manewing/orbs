#include "config.h"

config_t global_config = {
  .skip               =      0,
  .seed               =      0,
  .orb_count          =     80,
  .orb_score          =   1500,
  .orb_mutation       =  10000,
  .orb_bodies         = {  '.',  'o',  'O' },
  .orb_scores         = { 2000, 4000, 8000 },
  .food_rate          =     30,
  .food_types         = {  '+',  '#' },
  .food_scores        = { 1000, 4000 }
};
