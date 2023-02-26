//
// Created by Jared Lantner on 2/11/23.
//
#include <string.h>
#include "record.h"
#include "catalog.h"



Record create_record(int num_fields, char ** values, Schema * schema, int table_idx) {
    /**
     * Record should be variable length
     */

    int sum_of_lengths = 0;
    for(int i = 0; i < num_fields; i++){
        sum_of_lengths += strlen(values[i]) + 1;
    }

    void * space = malloc(4 * num_fields * 2 + 1 + sum_of_lengths) ;

    int * offsets = (int *)space;
    int * lengths = (offsets + 4 * num_fields);
    char * null_bitmap = (char *)space + 4 * num_fields * 2;
    char ** fields = (char **) null_bitmap + 1;
    int offset = 4*num_fields*2+1;
    for(int i = 0; i < num_fields; i++){
        offsets[i] = offset;
        lengths[i] = strlen(values[i]) + 1;
        strcpy((char *)(offsets[i] + offsets), values[i]);
        offset += lengths[i];
    }
    Record record = {offsets, lengths, null_bitmap, fields};
    return record;
}

