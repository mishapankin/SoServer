LFLAGS = -lm
CFLAGS = -std=gnu11 -g -Wall

C_FILES = $(wildcard src/*.c)
H_FILES = $(wildcard src/*.h)

TARGET = ./soserver

all: $(TARGET)

clean:
	rm -rf $(TARGET) *.o

run: $(TARGET)
	$(TARGET)

$(TARGET): $(C_FILES) $(H_FILES)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

.PHONY: all clean run