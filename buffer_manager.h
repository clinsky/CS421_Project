//
// Created by Jared Lantner on 2/26/23.
//

#ifndef CS421_PROJECT_BUFFER_MANAGER_H
#define CS421_PROJECT_BUFFER_MANAGER_H

#include "page.h"

struct pageBuffer {
    int num_pages;
    Page * buffer;
    int * in_memory;
    bool * modified;
    int * page_numbers;
};

typedef struct pageBuffer PageBuffer;

void purge_buffer();
void insert_record_into_table_file(char * db_loc, int table_idx, Record rec, Schema * schema, char * table_file, PageBuffer pageBuffer);
Page * request_page(int page_num, Schema * schema, char * table_name, char * db_loc, int table_idx, PageBuffer pageBuffer);
PageBuffer createPageBuffer(Schema * schema);
bool insert_at_end_of_page(Record rec, Page * page, Schema * schema, int table_idx);
#endif //CS421_PROJECT_BUFFER_MANAGER_H
