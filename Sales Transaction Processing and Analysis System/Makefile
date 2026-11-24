CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = sales
SOURCES = main.c
HEADERS = sales.h

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)
