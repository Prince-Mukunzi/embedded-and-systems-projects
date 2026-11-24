CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = sales
SOURCES = main.c input.c
HEADERS = sales.h input.h

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)
