CC = gcc
CFLAGS = -Wall
SRCS = main.c hashTable.c zipList.c

# The executable name
EXECUTABLE = my_program

all: $(EXECUTABLE)

$(EXECUTABLE): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(EXECUTABLE)

.PHONY: all clean