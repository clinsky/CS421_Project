#include "page.h"
#include "catalog.h"
#include <string.h>
#include <dirent.h>




char * create_new_table_file(char * table_file){
    return table_file;

}

FILE * get_table_file(char * db_loc, int table_idx){
    char char_string[2] = {table_idx, '\n'};
    char * table_filename = strcat(db_loc, "/table/");
    table_filename = strcat(table_filename, char_string);
    FILE * file = fopen(table_filename, "rb");
    return file;
}

Page new_page(Schema * schema) {
    int num_records = 1;
    void *free_space = malloc(schema->page_size);
    free_space += 4;
    int * num_records_ptr = (int *)(free_space - 4);
    *(num_records_ptr) = 0;
    int *offsets = free_space;
    int *primary_keys = free_space;
    Record *records = free_space + (schema->page_size - 4);
    Page page = {num_records_ptr, offsets, primary_keys, free_space, records};
    return page;
}




int get_page_number_location(int page_number, Schema * schema){
    return schema->page_locations[page_number];
}

void update_catalog(Schema * schema, int page_number, int page_location){
    schema->page_locations[page_number] = page_location;
}


ATTRIBUTE_TYPE get_primary_key(Record record){
    return INTEGER;
}

bool record_before_current_record(Record rec, Record current_record) {
    return get_primary_key(rec) < get_primary_key(current_record);
}

bool insert_at_end_of_page(Record rec, Page page) {
    int size_of_record = record_size(rec);
    if(page.records - size_of_record < page.free_space) {
        return true;
    }
    page.free_space += 8;
    page.primary_keys[*(page.num_records) + 2] = get_primary_key(rec);
    for(int i = *(page.num_records) - 1; i >= 0; i--){
        page.primary_keys[i + 2] = page.primary_keys[i];
    }
    page.primary_keys += 4;
    page.offsets[*(page.num_records) + 1] = ((int)((void *)page.records - (void *)&page)) - record_size(rec);
    for(int i = *(page.num_records) - 1; i >= 0; i--){
        page.offsets[i + 1] = page.offsets[i];
    }
    page.records -= record_size(rec);
    *page.records = rec;
    (*page.num_records)++;
    return false;
}

bool insert_before_current_record(Record rec, Record curr, Page page, int current_record_idx) {
    int size_of_record = record_size(rec);
    if(page.records - size_of_record < page.free_space) {
        return true;
    }
    page.free_space += 8;
    for (int i = *page.num_records; i > current_record_idx; i--) {
        page.primary_keys[i + 2] = page.primary_keys[i];
    }
    page.primary_keys[current_record_idx] = get_primary_key(rec);
    for (int i = current_record_idx - 1; i >= 0; i--) {
        page.primary_keys[i + 2] = page.primary_keys[i];
    }
    page.primary_keys += 4;
    for (int i = *page.num_records; i > current_record_idx; i--) {
        page.offsets[i + 1] = page.offsets[i];
    }
    page.offsets[current_record_idx] = ((int)((void *)page.records - (void *)&page)) - record_size(rec);
    for (int i = current_record_idx - 1; i >= 0; i--) {
        page.offsets[i + 1] = page.offsets[i];
    }
    page.records -= record_size(rec);
    *page.records = rec;
    return false;
}


void split_page(Page * page, FILE * table_file_ptr, Schema * schema, int page_number, int page_location){
    /*
     * make a new page
     * remove half the items from the current page
     * add the items to the new page
     * insert the new page after the current page in the table file
     */
    int global_page_size = schema->page_size;
    // make a new page
    Page new_page = new_page(schema);

    // remove half the items from the current page
    // add the items to the new page
    int left_num_records = page.num_records / 2;
    int right_num_records = page.num_records - left_num_records;
    for(int idx = 0; idx < right_num_records; idx++){
        new_page.offsets[idx] = page.offsets[idx + left_num_records];
        //new_page->records[idx - left_num_records] = page->records[idx];
        //new_page->num_records++;
    }



    new_page.primary_keys += 4*right_num_records;

    for(int idx = 0; idx < right_num_records; idx++){
        new_page.primary_keys[idx] = page.primary_keys[idx + left_num_records];
    }
    new_page.free_space += 4*(2*right_num_records);
    for(int idx = 0; idx < right_num_records; idx++){
        Record * record_ptr = (Record *)((void *)page.num_records + (void *)(page.offsets[idx+left_num_records]));
        Record record = *record_ptr;
        new_page.records -= new_page.offsets[idx];
        new_page.records[0] = record;
    }

    page.offsets -= 4*right_num_records;
    page.primary_keys -= 4*right_num_records;
    page.free_space -= 4*(2*right_num_records);
    *(page.num_records) = left_num_records;
    *(new_page.num_records) = right_num_records;
    buffer_and_write_page(new_page);




    // insert the new page after the current page in the table file
    fwrite(page, sizeof(page), 1, table_file_ptr);

    // update the schema
    update_catalog(schema, page_number, page_location);
}

void insert_record_into_table_file(char * db_loc, int table_idx, Record rec, Schema * schema){
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
    FILE * table_file_ptr = get_table_file(db_loc, table_idx);
    int * num_pages_ptr;
    fread(&num_pages_ptr, sizeof(int), 1, table_file_ptr);
    // if there are no pages for this table
    if(!num_pages_ptr){
        // make a new file for the table
        Page new_page = new_page(schema);
        int new_size = 1;
        //fwrite(&new_size, sizeof(int), 1, table_file_ptr);
        // add this entry to a new page
        //Page * new_page = (Page*)malloc(global_page_size);
        new_page.free_space += 4;


        // TODO: implement get_size_of_record in record.c
        int record_size = get_size_of_record(rec);
        *(new_page.offsets) = schema->page_size - record_size;
        *(new_page.primary_keys) = get_primary_key(rec);
        new_page.records -= record_size;
        *(new_page.records) = rec;


        *(new_page.num_records)++;
        //TODO: implement buffer_and_write_page in memory_manager.c
        buffer_and_write_page(new_page);
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
        //TODO: implement get_page_from_memory_manager in memory_manager.c
        page = get_page_from_memory_manager(table_file_ptr, page_number, schema);

        // Iterate the records in page

        for (int record_idx = 0; record_idx < page->num_records; record_idx++) {
            Record current_record = (page->records)[record_idx];
            if (record_before_current_record(rec, current_record)) {
                overfill = insert_before_current_record(rec, current_record, page, record_idx);
            }
        }
        if (overfill) {
            split_page(page, table_file_ptr, schema, page_number, *num_pages_ptr);
            buffer_and_write_page(page);
        }
        else{
            inserted = 1; // record was inserted
            buffer_and_write_page(page);
        }
        page_number++;
    }

    // if record not inserted, insert into last page of the file
    if(inserted == 0){
        page = get_page_from_memory_manager(table_file_ptr, page_number, schema);
        overfill = insert_at_end_of_page(rec, page);
        if(overfill){
            split_page(page, table_file_ptr, schema, *num_pages_ptr, *num_pages_ptr);
            buffer_and_write_page(page);
            page = get_page_from_memory_manager(table_file_ptr, page_number, schema);
            insert_at_end_of_page(rec, page);
            buffer_and_write_page(page);
        }
        else{
            buffer_and_write_page(page);
        }
    }
}
