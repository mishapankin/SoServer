LFLAGS = -lm
CFLAGS = -std=gnu11 -g -Wall

C_FILES = $(wildcard src/*.c)
H_FILES = $(wildcard src/*.h)

all: prog

clean:
	rm -r prog *.o

run: prog
	./prog

prog: $(C_FILES) $(H_FILES)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

.PHONY: all clean run