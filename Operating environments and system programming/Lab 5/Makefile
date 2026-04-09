CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread
TARGET = multi_thread_sort
SRC = multi_thread_sort.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean