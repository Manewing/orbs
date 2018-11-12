.PHONY: all clean

CFLAGS := -ggdb -Iinc/

SRCS := $(wildcard src/*.c)
OBJS := $(patsubst %.c, build/%.o, $(notdir $(SRCS)))
LIBS := -lpthread -lreadline

TESTS := orb_test
TEST_OBJS := $(filter-out build/main.o, $(OBJS))

all: orbs $(TESTS)

build:
	echo $(TEST_OBJS)
	mkdir -p build/test

orbs: $(OBJS) | build
	gcc $(CFLAGS) $(OBJS) -o orbs $(LIBS)

orb_test: $(TEST_OBJS) build/test/orb_test.o
	gcc $(CFLAGS) $(TEST_OBJS) build/test/orb_test.o -o test/orb_test $(LIBS)

build/%.o: src/%.c | build
	gcc $(CFLAGS) -c $< -o $@

build/test/%.o: test/%.c | build
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm -rf build orbs
