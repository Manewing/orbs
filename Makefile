.PHONY: all clean

CFLAGS := -ggdb -Iinc/

SRCS := $(wildcard src/*.c)
OBJS := $(patsubst %.c, build/%.o, $(notdir $(SRCS)))

all: orbs

build:
	mkdir -p build

orbs: $(OBJS) | build
	gcc $(CFLAGS) $(OBJS) -o orbs -lpthread -largp

build/%.o: src/%.c | build
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm -rf build orbs
