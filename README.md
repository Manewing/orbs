# ORBS

*ORBS* is a console based life simulator of *orb* entities written in C. It combines a simple 8-bit instruction set and respective simulator with a genetic algorithm.

## The simulation
Each *orb* ('.', 'o', 'O') is essentially a little micro-processor with registers and status flags.
The genome of the *orb* is it's instruction memory ("Havard architecture").
The world it is living in consists of other *orbs* and food ('+', '#').
If an *orb* hits food it eats the food automatically, which increases its *score*.
Every, by the *orb*, executed instruction reduces it's *score* by a given amount. The *score* of an *orb* reaches 0 it dies.
The *score* of an *orb* also determines whether it is able to reproduce itself or not.
When two *orbs* meet, they reproduce if they are able to. Reproduction is performed by the genetic algorithm, creating two new *orbs*.

## The simulator
The simulator is console based and offers different levels of interaction.
The simulation can be paused by pressing the `p` key.
This opens a shell allowing a more detailed analyses like the disassembly of the genome of an *orb*.
With `w` and `s` the simulation speed can be changed.
The different parameters of the simulation like the mutation rate, can be configured via the command line or by providing a configuration file.
For a documentation of the configurable parameters run:
```
./orbs --config-info
```

# Installation and build

## Prerequisites

The GNU C libraries readline and argp.

## Build

Just run `make`.
