#ifndef PAGE_H
#define PAGE_H

#include "catalog.h"
#include "record.h"
#include "table.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct page {
  int max_size;      // maxsize of page in bytes
  int page_number;   // which page
  int last_accessed; // buffer updates this (never gets written to file)
  int num_records;
  int total_bytes_from_records; // total bytes the records make up
  Record *records;
  struct page *next_page;
};

typedef struct page Page;

Attribute_Values *check_valid_parsed_tuple(Table *table,
                                           char (*tuple_parsed)[50]);

Page *add_record_to_page(Schema *schema, Table *table,
                         Attribute_Values *attr_val);

int calculate_record_size(Table *table, Attribute_Values *attr_vals);

Page *read_page_from_file(Schema *schema, char *file_path);

Page *create_first_page(Table *t, char (*tuple_parsed)[50]);

bool check_enough_space(Table *table, Page *p, Attribute_Values *attr_vals);

void write_page_to_file(Table *table, Page *p, char *file_path);

#endif
