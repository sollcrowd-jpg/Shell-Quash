CC = gcc
CFLAGS = -Wall -g -std=c99
TARGET = quash

all: $(TARGET)

$(TARGET): shell.c
	$(CC) $(CFLAGS) -o $(TARGET) shell.c

clean:
	rm -f $(TARGET)
