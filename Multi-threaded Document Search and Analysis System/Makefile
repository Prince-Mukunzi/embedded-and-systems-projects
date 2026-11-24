CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread
TARGET = document_analyzer
SOURCES = main.c setup.c
HEADERS = analyzer.h setup.h

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)
