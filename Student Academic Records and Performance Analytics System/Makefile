CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = records
SOURCES = main.c
HEADERS = records.h

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)
