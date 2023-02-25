#include "page.h"
#include "catalog.h"

int get_page_index(PageBuffer *page_buffer, Schema *schema){
    for(int i=0; i < schema->buffer_size; i++){
        if(page_buffer->pages[i].table_name == NULL){
            printf("Index is %d\n", i);
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
    
    printf("Index is %d\n", page_index);
    return page_index;
}