CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = sales
SOURCES = main.c input.c utils.c transaction.c operations.c filter.c
HEADERS = sales.h input.h utils.h transaction.h operations.h filter.h

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)
