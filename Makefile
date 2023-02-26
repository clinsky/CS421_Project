CC = gcc
CFLAGS = -c -Wall

all: main

main: main.o catalog.o buffer_manager.o page.o record.o parse_utils.o query_processor.o display.o
	$(CC) main.o catalog.o buffer_manager.o page.o record.o parse_utils.o query_processor.o display.o -o main


main.o: main.c
	$(CC) $(CFLAGS) main.c

buffer_manager.o: buffer_manager.c buffer_manager.h
	$(CC) $(CFLAGS) buffer_manager.c

page.o: page.c page.h
	$(CC) $(CFLAGS) page.c

record.o: record.c record.h
	$(CC) $(CFLAGS) record.c

parse_utils.o: parse_utils.c parse_utils.h
	$(CC) $(CFLAGS) parse_utils.c

query_processor.o: query_processor.c query_processor.h
	$(CC) $(CFLAGS) query_processor.c

display.o: display.c display.h
	$(CC) $(CFLAGS) display.c

catalog.o: catalog.c catalog.h
	$(CC) $(CFLAGS) catalog.c




clean:
	rm -f *.o main

