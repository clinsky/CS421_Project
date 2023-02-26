#include "page.h"
#include "catalog.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool bit_is_on(unsigned int num, int n) {
    unsigned int mask = 1u << n;
    return (num & mask) != 0;
}

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
        int offset_length_pairs[num_records][2];
        for(int i = 0; i < num_records; i++){
            fread(&offset_length_pairs[i][0], sizeof(int), 1, fp);
            fread(&offset_length_pairs[i][1], sizeof(int), 1, fp);
            new_page->num_bytes += offset_length_pairs[i][1];
        }
        
        // Make room for record values
        new_page->records = (char ***)malloc(num_records * sizeof(char **));
        for (int i = 0; i < table->num_attributes; i++) {
            new_page->records[i] = (char **)malloc(table->num_attributes * sizeof(char *));
        }
        
        // Iterate through records
        for(int i = 0; i < num_records; i++){
            // Iterate through offset pairs of attributes
            int attribute_offset_length_pairs[num_records][2];
            for(int j = 0; j < table->num_attributes; j++){
                fread(&attribute_offset_length_pairs[i][0], sizeof(int), 1, fp);
                fread(&attribute_offset_length_pairs[i][1], sizeof(int), 1, fp);
            }
            
            // Read null bit map
            unsigned int null_bit_map;
            fread(&null_bit_map, sizeof(unsigned int), 1, fp);
            
            // Iterate through attribute values
            for(int j = 0; j < table->num_attributes; j++){
                char *attribute;
                if(bit_is_on(null_bit_map, j)){
                    if (table->attributes[i].type == CHAR || table->attributes[i].type == VARCHAR){
                        attribute = malloc(attribute_offset_length_pairs[j][1]);
                        fread(attribute, attribute_offset_length_pairs[j][1], 1, fp);
                    }
                    else if (table->attributes[i].type == INTEGER){
                        int int_attribute;
                        fread(&int_attribute, attribute_offset_length_pairs[j][1], 1, fp);
                        sprintf(attribute, "%d", int_attribute);
                    }
                    else if (table->attributes[i].type == DOUBLE){
                        double double_attribute;
                        fread(&double_attribute, attribute_offset_length_pairs[j][1], 1, fp);
                        sprintf(attribute, "%f", double_attribute);
                    }
                    else if (table->attributes[i].type == BOOL){
                        bool bool_attribute;
                        fread(&bool_attribute, attribute_offset_length_pairs[j][1], 1, fp);
                        test = malloc(strlen("Hello") + 1); // Allocate memory for the string
                        strcpy(test, "Hello"); // Copy the characters into the allocated memory
                    }
                }
                else{
                
                }
            }
        }
        
        
        // Skip n bytes
        page_index++;
    }
}