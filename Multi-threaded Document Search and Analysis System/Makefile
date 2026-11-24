CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread
TARGET = document_analyzer
SOURCES = main.c setup.c search.c report.c thread.c
HEADERS = analyzer.h setup.h search.h report.h thread.h

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)
