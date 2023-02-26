#include "page.h"
#include "buffer_manager.h"
#include "catalog.h"
#include "record.h"
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

Page * new_page(Schema * schema) {
    int num_records = 1;
    void *free_space = malloc(schema->page_size);
    free_space += 4;
    int * num_records_ptr = (int *)(free_space - 4);
    *(num_records_ptr) = 0;
    int *offsets = free_space;
    int *primary_keys = free_space;
    Record *records = free_space + (schema->page_size - 4);
    Page * page = malloc(sizeof(Page));
    page->num_records = num_records_ptr;
    page->offsets = offsets;
    page->primary_keys = primary_keys;
    page->free_space = free_space;
    page->records = records;
    return page;
    // Page page = {num_records_ptr, offsets, primary_keys, free_space, records};
    return page;
}




int get_page_number_location(int page_number, Schema * schema){
    return schema->page_locations[page_number];
}

void update_catalog(Schema * schema, int page_number, int page_location){
    schema->page_locations[page_number] = page_location;
}


char * get_primary_key(Record record){
    return record.data[0];
}

bool record_before_current_record(Record rec, Record current_record) {
    return get_primary_key(rec) < get_primary_key(current_record);
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
    page->offsets[*(page->num_records) + 1] = ((int)((void *)page->records - (void *)&page)) - record_size(rec, schema, table_idx);
    for(int i = *(page->num_records) - 1; i >= 0; i--){
        page->offsets[i + 1] = page->offsets[i];
    }
    page->records -= record_size(rec, schema, table_idx);
    *(page->records) = rec;
    *(page->num_records)++;
    return false;
}

bool insert_before_current_record(Record rec, Record curr, Page * page, int current_record_idx, Schema * schema, int table_idx) {
    int size_of_record = record_size(rec, schema, table_idx);
    if(page->records - size_of_record < page->free_space) {
        return true;
    }
    page->free_space += 8;
    for (int i = *(page->num_records); i > current_record_idx; i--) {
        page->primary_keys[i + 2] = page->primary_keys[i];
    }
    page->primary_keys[current_record_idx] = get_primary_key(rec);
    for (int i = current_record_idx - 1; i >= 0; i--) {
        page->primary_keys[i + 2] = page->primary_keys[i];
    }
    page->primary_keys += 4;
    for (int i = *page->num_records; i > current_record_idx; i--) {
        page->offsets[i + 1] = page->offsets[i];
    }
    page->offsets[current_record_idx] = ((int)((void *)page->records - (void *)&page)) - record_size(rec, schema, table_idx);
    for (int i = current_record_idx - 1; i >= 0; i--) {
        page->offsets[i + 1] = page->offsets[i];
    }
    page->records -= record_size(rec, schema, table_idx);
    *(page->records) = rec;
    return false;
}

void write_to_file(Page * page, char * table_name, Schema * schema, int page_num, char * db_loc, int table_idx){
    char * path = db_loc;
    path = strcat(path, "/table/");
    path = strcat(path, table_name);
    FILE * fp = fopen(path, "rb");
    fwrite(page->num_records, 4, 1, fp);
    for(int i = 0; i < *(page->num_records); i++){
        fwrite(page->offsets[i], 4, 1, fp);
    }
    for(int i = 0; i < *page->num_records; i++){
        fwrite(page->primary_keys[i], 4, 1, fp);
    }
    void * current_location = page->free_space;
    while(current_location < page->records){
        fwrite(current_location, 1, 1, fp);
        current_location += 1;
    }
    for(int i = 0; i < *(page->num_records); i++){
        Record record = page->records[i];
        //TODO: write record to file
        for(int i = 0; i < *(page->num_records); i++){
            fwrite(record.offsets[i], 4, 1, fp);
        }
        for(int i = 0; i < *(page->num_records); i++){
            fwrite(record.lengths[i], 4, 1, fp);
        }
        fwrite(record.null_array, 1, 1, fp);
        for(int i = 0; i < *(page->num_records); i++){
            fwrite(record.data[i], record.lengths[i], 1, fp);
        }
    }
}

void split_page(Page * page, FILE * table_file_ptr, Schema * schema, int page_number, int page_location, char * table_file, char * db_loc, int table_idx){
    /*
     * make a new page
     * remove half the items from the current page
     * add the items to the new page
     * insert the new page after the current page in the table file
     */
    int global_page_size = schema->page_size;
    // make a new page
    Page * newPage = new_page(schema);

    // remove half the items from the current page
    // add the items to the new page
    int left_num_records = *(page->num_records) / 2;
    int right_num_records = page->num_records - left_num_records;
    for(int idx = 0; idx < right_num_records; idx++){
        newPage->offsets[idx] = page->offsets[idx + left_num_records];
        //new_page->records[idx - left_num_records] = page->records[idx];
        //new_page->num_records++;
    }



    newPage->primary_keys += 4*right_num_records;

    for(int idx = 0; idx < right_num_records; idx++){
        newPage->primary_keys[idx] = page->primary_keys[idx + left_num_records];
    }
    newPage->free_space += 4*(2*right_num_records);
    for(int idx = 0; idx < right_num_records; idx++){
        Record * record_ptr = (Record *)(page->num_records + page->offsets[idx+left_num_records]);
        Record record = *record_ptr;
        newPage->records -= newPage->offsets[idx];
        newPage->records[0] = record;
    }

    page -> offsets -= 4*right_num_records;
    page->primary_keys -= 4*right_num_records;
    page->free_space -= 4*(2*right_num_records);
    *(page->num_records) = left_num_records;
    page->num_records = right_num_records;

    // void write_to_file(Page page, char * table_name, Schema * schema, int page_num, char * db_loc, int table_idx)
    write_to_file(newPage, schema->tables[table_idx].name, schema, page_number + 1, db_loc, table_idx);
    // insert the new page after the current page in the table file
    //fwrite(page, sizeof(page), 1, table_file_ptr);
    // update the schema
    //update_catalog(schema, page_number, page_location);
}

/*
void insert_record_into_table_file(char * db_loc, int table_idx, Record rec, Schema * schema, char * table_file, PageBuffer pageBuffer){

    int global_page_size = schema->page_size;
    FILE * table_file_ptr = get_table_file(db_loc, table_idx);
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
        page = request_page(page_number, schema, pageBuffer, schema->tables[table_idx].name, db_loc, table_idx);

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
        page = request_page(page_number, schema, pageBuffer, schema->tables[table_idx].name, db_loc, table_idx);
        overfill = insert_at_end_of_page(rec, page, schema, table_idx);
        if(overfill){
            split_page(&page, table_file_ptr, schema, page_number, *num_pages_ptr, table_file, db_loc, table_idx);
            page = request_page(page_number, schema, pageBuffer, schema->tables[table_idx].name, db_loc, table_idx);
            insert_at_end_of_page(rec, page, schema, table_idx);
        }
    }
}
*/
