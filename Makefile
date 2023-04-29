CC = gcc
CFLAGS = -c -Wall

all: main

main: main.o catalog.o page.o bplus_tree.o record.o conditional_parser.o parse_utils.o query_processor.o display.o
	$(CC) main.o catalog.o page.o bplus_tree.o record.o conditional_parser.o parse_utils.o query_processor.o display.o -o main -lm

main.o: main.c
	$(CC) $(CFLAGS) main.c

page.o: page.c page.h
	$(CC) $(CFLAGS) page.c

bplus_tree.o: bplus_tree.c bplus_tree.h
	$(CC) $(CFLAGS) bplus_tree.c

record.o: record.c record.h
	$(CC) $(CFLAGS) record.c

conditional_parser.o: conditional_parser.c conditional_parser.h
	$(CC) $(CFLAGS) conditional_parser.c

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

