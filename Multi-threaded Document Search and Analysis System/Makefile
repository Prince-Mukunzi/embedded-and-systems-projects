CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread
TARGET = document_analyzer
SOURCES = main.c
HEADERS = analyzer.h

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)
