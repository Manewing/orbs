#include "config.h"

global_config_t global_config = {.skip = 0,
                                 .seed = 0,
                                 .herz = 201,
                                 .orb_count = 80,
                                 .orb_score = 1500,
                                 .orb_ttl = 0xffffff,
                                 .orb_mutation = 10000,
                                 .orb_bodies = {'.', 'o', 'O'},
                                 .orb_scores = {2000, 4000, 8000},
                                 .food_rate = 30,
                                 .food_types = {'+', '#'},
                                 .food_scores = {1000, 4000},
                                 .stats_output = "\0"};

static config_elem_t global_config_elems[] = {
    CONFIG_ELEM(global_config_t, global_config, "%d", skip,
                "Skip first #skip iterations"),
    CONFIG_ELEM(global_config_t, global_config, "%d", herz,
                "The frame rate in Hz"),
    CONFIG_ELEM(global_config_t, global_config, "%d", seed,
                "Seed for srand (default = 0)"),
    CONFIG_ELEM(global_config_t, global_config, "%d", orb_count,
                "Initial count of orbs"),
    CONFIG_ELEM(global_config_t, global_config, "%d", orb_score,
                "Initial score of orb"),
    CONFIG_ELEM(global_config_t, global_config, "%d", orb_ttl,
                "(Max) Time to live of orb"),
    CONFIG_ELEM(global_config_t, global_config, "%d", orb_mutation,
                "Mutation rate of orbs P(mutate) = 1 / orb_mutation"),
    CONFIG_ELEM(global_config_t, global_config, "%d", orb_scores[0],
                "Min. score for orb type '.'"),
    CONFIG_ELEM(global_config_t, global_config, "%d", orb_scores[1],
                "Min. score for orb type 'o'"),
    CONFIG_ELEM(global_config_t, global_config, "%d", orb_scores[2],
                "Min. score for orb type 'O'"),
    CONFIG_ELEM(global_config_t, global_config, "%d", food_rate,
                "Food spawn rate P(food) = 1 / food_rate"),
    CONFIG_ELEM(global_config_t, global_config, "%d", food_scores[0],
                "Score of food '+'"),
    CONFIG_ELEM(global_config_t, global_config, "%d", food_scores[1],
                "Score of food '#'"),
    CONFIG_ELEM(global_config_t, global_config, "%s", stats_output,
                "Output directory for statistics")};

config_reader_t global_config_reader = {
    .elems = global_config_elems,
    .elem_count = sizeof(global_config_elems) / sizeof(config_elem_t)};
