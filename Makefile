CC = gcc
CFLAGS = -c -Wall

all: main

main: main.o page.o parse_utils.o query_processor.o display.o
	$(CC) main.o page.o parse_utils.o query_processor.o display.o catalog.o -o main

main.o: main.c
	$(CC) $(CFLAGS) main.c

catalog.o: catalog.c catalog.h
	$(CC) $(CFLAGS) catalog.c

page.o: page.c page.h
	$(CC) $(CFLAGS) page.c

parse_utils.o: parse_utils.c parse_utils.h
	$(CC) $(CFLAGS) parse_utils.c

query_processor.o: query_processor.c query_processor.h
	$(CC) $(CFLAGS) query_processor.c

display.o: display.c display.h
	$(CC) $(CFLAGS) display.c




clean:
	rm -f *.o main

