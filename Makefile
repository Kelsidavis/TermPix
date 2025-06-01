CC = gcc
CFLAGS = -Wall -O2
LDFLAGS =

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

all: termpix

termpix: $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f termpix src/*.o
