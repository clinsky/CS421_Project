#ifndef PAGE_H
#define PAGE_H

#include <stdio.h>
#include <stdlib.h>
#include "record.h"
#include "catalog.h"

struct page {
  int * num_records;
  int * offsets;
  int * primary_keys;
  void * free_space;
  Record * records;
};

typedef struct page Page;

Page *page_splitting(int *keys, int num_keys, int max_num_records);
void split_page(Page * page, FILE * table_file_ptr, Schema * schema, int page_number, int page_location, char * table_name, char * db_loc, int table_idx);
void insert_key_at_end_of_page(Page *page_ptr, int pkey);
void insert_key_into_page_location(Page *page_ptr, int pkey, int count);
void insert_key_into_page(Page *page_ptr, int pkey);
Page * new_page(Schema * schema);
//void insert_record_into_table_file(char * db_loc, int table_idx, Record rec, Schema * schema, char * table_file, PageBuffer pageBuffer);
void write_to_file(Page * page, char * table_name, Schema * schema, int page_num, char * db_loc, int table_idx);
bool record_before_current_record(Record record, Record current_record);
bool insert_before_current_record(Record record, Record current_record, Page * page, int record_idx, Schema * schema, int table_idx);
char * get_primary_key(Record record);
bool insert_at_end_of_page(Record rec, Page * page, Schema * schema, int table_idx);
#endif
