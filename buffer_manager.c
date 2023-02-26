//
// Created by Jared Lantner on 2/26/23.
//
#include "buffer_manager.h"
#include "page.h"
#include <string.h>

PageBuffer createPageBuffer(Schema * schema){
    int num_pages = 0;
    Page * buffer = (Page *)malloc(schema->buffer_size);
    int * in_memory = (int *)malloc(schema->buffer_size / schema->page_size);
    bool * modified = (bool *)malloc(schema->buffer_size / schema->page_size);
    int * page_numbers = (int *)malloc(schema->buffer_size / schema->page_size);
    PageBuffer pageBuffer = {num_pages, buffer, in_memory, modified, page_numbers};
    return pageBuffer;
}

Page read_page_from_disk(int page_num, Schema * schema, PageBuffer pageBuffer, char * table_name, char * db_loc, int table_idx){
    Page page = new_page(schema);
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
        rewind(fp);
    }
    page->records = (Record *)(page->num_records + min_offset);
    fclose(fp);
    return *page;
}

Page request_page(int page_num, Schema * schema, char * table_name, char * db_loc, int table_idx, PageBuffer pageBuffer){
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
        //read_page_from_disk(int page_num, Schema * schema, PageBuffer pageBuffer, char * table_name, char * db_loc, int table_idx){
        pageBuffer.buffer[pageBuffer.num_pages] = read_page_from_disk(page_num, schema, pageBuffer, table_name, db_loc, table_idx);
        pageBuffer.num_pages++;
        return pageBuffer.buffer[pageBuffer.num_pages - 1];
    }
    else {
        write_to_file(pageBuffer.buffer[0], table_name, schema, page_num, db_loc, table_idx);
        for(int i = 0; i < pageBuffer.num_pages; i++) {
            pageBuffer.buffer[i] = pageBuffer.buffer[i + 1];
        }
        // read_page_from_disk(int page_num, Schema * schema, PageBuffer pageBuffer, char * table_name, char * db_loc, int table_idx)
        pageBuffer.buffer[0] = read_page_from_disk(page_num, schema, pageBuffer, table_name, db_loc, table_idx);
        pageBuffer.page_numbers[0] = page_num;
        pageBuffer.modified[0] = false;
        pageBuffer.in_memory[page_num] = 0;
    }
}

bool insert_at_end_of_page(Record rec, Page * page, Schema * schema, int table_idx) {
    int size_of_record = record_size(rec, schema, table_idx);
    if(page->records - size_of_record < page->free_space) {
        return true;
    }
    page->free_space += 8;
    page->primary_keys[*(page->num_records) + 2] = get_primary_key(rec);
    for(int i = *(page->num_records) - 1; i >= 0; i--){
        page->primary_keys[i + 2] = page->primary_keys[i];
    }
    page->primary_keys += 4;
    page->offsets[*(page.num_records) + 1] = ((int)((void *)page.records - (void *)&page)) - record_size(rec, schema, table_idx);
    for(int i = *(page.num_records) - 1; i >= 0; i--){
        page.offsets[i + 1] = page.offsets[i];
    }
    page.records -= record_size(rec, schema, table_idx);
    *page.records = rec;
    (*page.num_records)++;
    return false;
}


void insert_record_into_table_file(char * db_loc, int table_idx, Record rec, Schema * schema, char * table_file, PageBuffer pageBuffer){
    /*
     * if there are no pages for this table:
     *  make a new file for the table
     *  add this entry to a new page
     *  insert the page into the table file
     *  end
     * Read each table page in order from the table file:
     *  iterate the records in page:
     *      if the record belongs before the current record:
     *          insert the record before it
     *  if the current page becomes overfull:
     *      split the page
     *      end
     * If the record does not get inserted:
     *  insert it into the last page of the table file
     *  if page becomes overfull:
     *      split the page
     *  end
     */
    int global_page_size = schema->page_size;
    //FILE * table_file_ptr = get_table_file(db_loc, table_idx);
    char * path = (char *)malloc(100);
    strcpy(path, db_loc);
    strcat(path, "/tables/");
    strcat(path, table_file);
    FILE * table_file_ptr = fopen(path, "rb");
    int * num_pages_ptr;
    fread(&num_pages_ptr, sizeof(int), 1, table_file_ptr);
    // if there are no pages for this table
    if(!num_pages_ptr){
        // make a new file for the table
        Page newPage = new_page(schema);
        int new_size = 1;
        //fwrite(&new_size, sizeof(int), 1, table_file_ptr);
        // add this entry to a new page
        //Page * new_page = (Page*)malloc(global_page_size);
        newPage.free_space += 4;

        int recordSize = record_size(rec, schema, table_idx);
        *(newPage.offsets) = schema->page_size - recordSize;
        *(newPage.primary_keys) = get_primary_key(rec);
        newPage.records -= recordSize;
        *(newPage.records) = rec;
        *(newPage.num_records)++;
        write_to_file(newPage, schema->tables[table_idx].name, schema, 0, db_loc, table_idx);
        return;
    }

    Page page;
    int inserted = 0;
    bool overfill = false;
    // Read each table page in order from the table file
    int page_number = 0;
    while(page_number < *num_pages_ptr){
        // read in the page

        //page = fseek(table_file_ptr, get_page_number_location(page_number, schema), 0);
        // Page request_page(int page_num, Schema * schema, PageBuffer pageBuffer, char * table_name, char * db_loc, int table_idx);
        page = request_page(page_number, schema, schema->tables[table_idx].name, db_loc, table_idx, pageBuffer);

        // Iterate the records in page

        for (int record_idx = 0; record_idx < *(page.num_records); record_idx++) {
            Record current_record = (page.records)[record_idx];
            if (record_before_current_record(rec, current_record)) {
                overfill = insert_before_current_record(rec, current_record, page, record_idx, schema, table_idx);
            }
        }
        if (overfill) {
            split_page(&page, table_file_ptr, schema, page_number, *num_pages_ptr, table_file, db_loc, table_idx);
        }
        else{
            inserted = 1; // record was inserted
        }
        page_number++;
    }

    // if record not inserted, insert into last page of the file
    if(inserted == 0){
        page = request_page(page_number, schema, schema->tables[table_idx].name, db_loc, table_idx, pageBuffer);
        overfill = insert_at_end_of_page(rec, page, schema, table_idx);
        if(overfill){
            split_page(&page, table_file_ptr, schema, page_number, *num_pages_ptr, table_file, db_loc, table_idx);
            page = request_page(page_number, schema, schema->tables[table_idx].name, db_loc, table_idx, pageBuffer);
            insert_at_end_of_page(rec, page, schema, table_idx);
        }
    }
}
