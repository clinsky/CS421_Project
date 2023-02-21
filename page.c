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

int get_page_number_location(int page_number, Schema * schema){
    return schema->page_locations[page_number];
}

void update_catalog(Schema * schema, int page_number, int page_location){
    schema->page_locations[page_number] = page_location;
}

bool record_before_current_record(Record rec, Record current_record) {
    return get_primary_key(rec) < get_primary_key(current_record);
}

void insert_before_current_record(Record rec, Record curr, Page * page, int current_record_idx) {
        Record temp = rec;
        for (int idx = current_record_idx; idx < page->num_records - 1; idx++) {
            page->records[idx + 1] = page->records[idx];
            page->records[idx] = temp;
            temp = page->records[idx+1];
        }
        (page->num_records)++;
}

bool page_is_overfull(Page * page) {
    return sizeof(page) > global_page_size;
}

void split_page(Page * page, FILE * table_file_ptr, Schema * schema, int page_number, int page_location){
    /*
     * make a new page
     * remove half the items from the current page
     * add the items to the new page
     * insert the new page after the current page in the table file
     */

    // make a new page
    Page * new_page = (Page *)malloc(global_page_size);
    new_page->num_records = 0;
    // remove half the items from the current page
    // add the items to the new page
    int left_num_records = page->num_records / 2;
    int right_num_records = page->num_records - left_num_records;
    for(int idx = left_num_records; idx < page->num_records; idx++){
        new_page->records[idx - left_num_records] = page->records[idx];
        new_page->num_records++;
    }
    page->num_records = left_num_records;

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

    FILE * table_file_ptr = get_table_file(db_loc, table_idx);
    int * num_pages_ptr;
    fread(&num_pages_ptr, sizeof(int), 1, table_file_ptr);
    // if there are no pages for this table
    if(!num_pages_ptr){
        // make a new file for the table
        int new_size = 1;
        fwrite(&new_size, sizeof(int), 1, table_file_ptr);
        // add this entry to a new page
        Page * new_page = (Page*)malloc(global_page_size);
        new_page->records[0] = rec;
        new_page->size++;
        // insert the page into the table file
        fwrite(new_page, global_page_size, 1, table_file_ptr);
        fclose(table_file_ptr);
        return;
    }

    Page * page;
    int inserted = 0;
    // Read each table page in order from the table file
    for(int page_number = 0; page_number <= *num_pages_ptr; page_number++){
        // read in the page

        page = fseek(table_file_ptr, get_page_number_location(page_number, schema));
        // Iterate the records in page
        for (int record_idx = 0; record_idx < page->num_records; record_idx++) {
            Record current_record = (page->records)[record_idx];
            if (record_before_current_record(rec, current_record)) {
                insert_before_current_record(rec, current_record, page, record_idx);
                inserted = 1;
            }
        }
        if (page_is_overfull(page)) {
            split_page(page, table_file_ptr, schema, page_number, *num_pages_ptr);


        }

    }
    // if record not inserted, insert into last page of the file
    if(inserted == 0){
        page->records[page->num_records + 1] = rec;
        if(page_is_overfull(page)){
            split_page(page, table_file_ptr, schema, *num_pages_ptr, *num_pages_ptr);
        }
    }
}
