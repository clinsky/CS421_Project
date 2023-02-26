#ifndef PAGE_H
#define PAGE_H

#include "attribute_types.h"
#include "catalog.h"
#include "record.h"
#include "table.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct offset {
  int page_offset; // where record starts
};

typedef struct offset Offset;

struct page {
  int max_size;      // maxsize of page in bytes
  int page_number;   // which page
  int last_accessed; // buffer updates this (never gets written to file)
  int num_records;
  int record_capacity; // UNRELATED TO PAGE/RECORD SIZE. USED SOLELY FOR
                       // REALLOCS
  int total_bytes_from_records; // total bytes the records make up
  Offset *offsets;
  Record *records;
  struct page *next_page;
};

typedef struct page Page;

struct buffer_entry {
  Page *page;
  char *table_name;
  char *file_path;
  Table *table;
  int last_used;
};

typedef struct buffer_entry Buffer_Entry;

// Schema
struct bufferm {
  int max_pages;
  int curr_pages;
  Buffer_Entry *entries;
  int counter;
};

typedef struct bufferm Bufferm;

Record *check_valid_parsed_tuple(Table *table, char (*tuple_parsed)[50]);

Page *add_record_to_page(Schema *schema, Table *table, Record *record,
                         Bufferm *buffer);

Page *insert_record_to_page(Schema *schema, Table *table, Page *p,
                            Record *record);

int calculate_record_size(Table *table, Record *record);

Page *read_page_from_file(Schema *schema, Table *table, char *file_path);

Page *create_first_page(Table *t, char (*tuple_parsed)[50]);

bool check_enough_space(Table *table, Page *p, Record *record);

void write_page_to_file(Table *table, Page *p, char *file_path);

void print_page(Table *table, Page *p);

void print_record(Table *table, Record *record);

bool is_page_overfull(Page *p);

void make_new_page_if_full(Page *p);

Bufferm *create_new_bufferm(int max_pages);

Page *search_buffer(Bufferm *b);

Page *find_in_buffer(Bufferm *b, Table *t);

void add_to_buffer(Bufferm *b, Table *table, Page *p, char *filepath);

void flush_buffer(Bufferm *b);

#endif
