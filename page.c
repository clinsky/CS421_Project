#include "page.h"
#include "catalog.h"

int write_page(int page_index, PageBuffer *page_buffer, Schema *schema){
    return 1;
}

int get_page_index(PageBuffer *page_buffer, Schema *schema){
    for(int i=0; i < schema->buffer_size; i++){
        if(page_buffer->pages[i].table_name == NULL){
            write_page(i, page_buffer, schema);
            return i;
        }
    }
    
    int min_last_used_count = page_buffer->last_used[0];
    int page_index = 0;
    for(int i=1; i < schema->buffer_size; i++){
        if(page_buffer->last_used[i] < min_last_used_count){
            min_last_used_count = page_buffer->last_used[i];
            page_index = i;
        }
    }
    
    write_page(page_index, page_buffer, schema);
    return page_index;
}

int read_table(char *table_name, PageBuffer *page_buffer, Schema *schema){
    Table *table = get_table(schema, table_name);
    char filepath[200];
    snprintf(filepath, sizeof(filepath), "%s/%s/%s", schema->db_path, "tables", table_name);
    FILE *fp = fopen(filepath, "rb");
    
    int page_index = 0;
    while(1){
        Page *new_page = (Page *)malloc(sizeof(Page));
        new_page->num_bytes = 0;
        new_page->table_name = table_name;
        new_page->page_index = page_index;
        new_page->records = NULL;
        
        // Read number of records
        int num_records;
        fread(&num_records, sizeof(int), 1, fp);
        new_page->num_records = num_records;
        
        // Read offset pairs of offset, length
        int offset_pairs[num_records][2];
        for(int i = 0; i < num_records; i++){
            fread(&offset_pairs[i][0], sizeof(int), 1, fp);
            fread(&offset_pairs[i][1], sizeof(int), 1, fp);
            new_page->num_bytes += + offset_pairs[i][1];
        }
        
        // Read values into page
        new_page->records = (char ***)malloc(num_records * sizeof(char **));
        for (int i = 0; i < table->num_attributes; i++) {
            new_page->records[i] = (char **)malloc(table->num_attributes * sizeof(char *));
        }
        
        // Skip n bytes
        page_index++;
    }
}