CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -g -lm
OBJECTS = main.o populate.o piggy_type.o select_sockets.o commands.o
EXES = main

piggy2: $(OBJECTS)
		$(CC) -o main $(LDFLAGS) $(OBJECTS) -lcurses

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXES)
