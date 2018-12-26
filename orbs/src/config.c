#include "config.h"
#include "orb.h"

global_config_t global_config = {
    .skip = 0UL, .exit = -1UL, .seed = 0, .herz = 201, .stats = "\0"};

static config_elem_t global_config_elems[] = {
    CONFIG_ELEM(global_config_t, global_config, "%lu", skip,
                "Skip first #skip iterations"),
    CONFIG_ELEM(global_config_t, global_config, "%lu", exit,
                "Exit after #exit iterations"),
    CONFIG_ELEM(global_config_t, global_config, "%d", herz,
                "The frame rate in Hz"),
    CONFIG_ELEM(global_config_t, global_config, "%d", seed,
                "Seed for srand (default = 0)"),
    CONFIG_ELEM(global_config_t, global_config, "%s", stats,
                "Output directory for statistics"),
    CONFIG_ELEM(orb_config_t, orb_config, "%d", count, "Initial count of orbs"),
    CONFIG_ELEM(orb_config_t, orb_config, "%d", score, "Initial score of orb"),
    CONFIG_ELEM(orb_config_t, orb_config, "%d", ttl,
                "(Max) Time to live of orb"),
    CONFIG_ELEM(orb_config_t, orb_config, "%d", mutation,
                "Mutation rate of orbs P(mutate) = 1 / mutation"),
    CONFIG_ELEM(orb_config_t, orb_config, "%d", scores[0],
                "Min. score for orb type '.'"),
    CONFIG_ELEM(orb_config_t, orb_config, "%d", scores[1],
                "Min. score for orb type 'o'"),
    CONFIG_ELEM(orb_config_t, orb_config, "%d", scores[2],
                "Min. score for orb type 'O'"),
    CONFIG_ELEM(map_config_t, map_config, "%d", food_rate,
                "Food spawn rate P(food) = 1 / food_rate"),
    CONFIG_ELEM(map_config_t, map_config, "%d", food_scores[0],
                "Score of food '+'"),
    CONFIG_ELEM(map_config_t, map_config, "%d", food_scores[1],
                "Score of food '#'")};

config_reader_t global_config_reader = {
    .elems = global_config_elems,
    .elem_count = sizeof(global_config_elems) / sizeof(config_elem_t)};
