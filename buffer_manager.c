//
// Created by Jared Lantner on 2/26/23.
//
#include "buffer_manager.h"
#include "page.h"
#include <string.h>

Page read_page_from_disk(int page_num, Schema * schema, PageBuffer pageBuffer, char * table_name, char * db_loc, int table_idx){
    Page * page = new_page(schema);
    char path[256];
    strcpy(path, db_loc);
    strcat(path, "/tables/");
    strcat(path, table_name);
    FILE * fp = fopen(path, "rb");
    fseek(fp, schema->tables[table_idx].page_locations[page_num]*schema->page_size, SEEK_SET);
    int num_records;
    fread(&num_records, sizeof(int), 1, fp);
    *(page->num_records) = num_records;
    for(int i = 0; i < num_records; i++){
        int offset;
        fread(&offset, sizeof(int), 1, fp);
        page->offsets[i] = offset;
    }
    page->primary_keys += 4 * num_records;
    for(int i = 0; i < num_records; i++){
        int primary_key;
        fread(&primary_key, sizeof(int), 1, fp);
        page->primary_keys[i] = primary_key;
    }
    page->primary_keys += 4*num_records*2;
    page->free_space+=4*num_records*2;
    int min_offset = schema->page_size + 1;
    for(int i = 0; i < num_records; i++){
        int current_offset = page->offsets[i];
        if(current_offset < min_offset){
            min_offset = current_offset;
        }
        fseek(fp, current_offset, SEEK_SET);
        //TODO: Read in the record from the file
        int num_attributes = schema->tables[table_idx].num_attributes;
        int * offsets = (int *)malloc(4 * num_attributes);
        for(int j = 0; j < num_attributes; j++){
            fread(&offsets[i], sizeof(int), 1, fp);
        }

        int * sizes = (int *)malloc(4 * num_attributes);
        for(int j = 0; j < num_attributes; j++){
            fread(&sizes[j], sizeof(int), 1, fp);
        }
        char * null_bitmap = (char *)malloc(1);
        fread(null_bitmap, sizeof(char), 1, fp);
        char ** attributes = (char **)malloc(8 * num_attributes);
        for(int j = 0; j < num_attributes; j++){
            attributes[j] = (char *)malloc(sizes[j]);
            fread(attributes[j], sizeof(char), sizes[j], fp);
        }
        Record record = create_record(num_records, attributes, schema, table_idx);
        *(Record *)(page->num_records + offsets[i]) = record;
    }
    page->records = (Record *)(page->num_records + min_offset);
    return *page;
}

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
        for(int i = 0; i < pageBuffer.num_pages; i++) {
            pageBuffer.buffer[i] = pageBuffer.buffer[i + 1];
        }
        pageBuffer.buffer[0] = read_page_from_disc(page_num, schema, pageBuffer);
        pageBuffer.page_numbers[0] = page_num;
        pageBuffer.modified[0] = false;
        pageBuffer.in_memory[page_num] = 0;
    }
}
