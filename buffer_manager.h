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
void request_page(int page_num);
void write_page_LRU();
Page read_page_from_disc(int page_num, Schema * schema, PageBuffer pageBuffer);





#endif //CS421_PROJECT_BUFFER_MANAGER_H
