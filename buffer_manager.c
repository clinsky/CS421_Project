//
// Created by Jared Lantner on 2/26/23.
//
#include "buffer_manager.h"
#include "page.h"

Page request_page(int page_num, Schema * schema, PageBuffer pageBuffer, char * table_name){
    /*
     * If page is in buffer, return it's address
     * Else, if there is space in the buffer, load the page into the buffer and return it's address
     * If no s;pace, write the LRU page to disk and load the page into the buffer and return it's address
     *
     */
    int idx = pageBuffer.in_memory[page_num];
    if(idx != -1){
        return pageBuffer.buffer[idx];
    }

    else if(schema->page_size * pageBuffer.num_pages + schema->page_size < schema->buffer_size) {
        pageBuffer.in_memory[pageBuffer.num_pages] = page_num;
        pageBuffer.modified[pageBuffer.num_pages] = false;
        pageBuffer.buffer[pageBuffer.num_pages] = read_page_from_disc(page_num, schema, pageBuffer, table_name);
        pageBuffer.num_pages++;
        return pageBuffer.buffer[pageBuffer.num_pages - 1];
    }
    else {
        write_page_LRU(pageBuffer);
        return read_page_from_disc(page_num, schema, pageBuffer);
    }









        }
        write_page_LRU(pageBuffer);
        pageBuffer.in_memory[0] = page_num;
        pageBuffer.modified[0] = false;
        pageBuffer.buffer[0] = load_page_from_disk(page_num, schema);
        return pageBuffer.buffer[0];
    }



}
