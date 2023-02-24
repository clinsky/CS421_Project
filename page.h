#ifndef PAGE_H
#define PAGE_H

#include <stdio.h>
#include <stdlib.h>
#include "record.h"
#include "catalog.h"

struct page {
  int num_records;
  Record * records;
};

typedef struct page Page;
void insert_record_into_table_file(char * db_loc, int table_idx, Record rec, Schema * schema);
Page *page_splitting(int *keys, int num_keys, int max_num_records);
void split_page(Page * page, FILE * table_file_ptr, Schema * schema, int page_number, int page_location);
void insert_key_at_end_of_page(Page *page_ptr, int pkey);
void insert_key_into_page_location(Page *page_ptr, int pkey, int count);
void insert_key_into_page(Page *page_ptr, int pkey);
Page *new_page(int max_num_records);

#endif
