CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic
TARGET = mydaemon
SRC = mydaemon.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) -c daemon.conf -l daemon.log -p mydaemon.pid

clean:
	rm -f $(TARGET) *.o daemon.log mydaemon.pid
