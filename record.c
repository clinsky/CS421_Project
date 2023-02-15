//
// Created by Jared Lantner on 2/11/23.
//
#include <string.h>
#include "record.h"



Record * init_record(int num_fields, Attribute * attributes, void * start, char null_array) {
    /**
     * Record should be variable length
     */
    int offset = 21;
    void * pair_location = start;
    for(int i = 0; i < num_fields; i++){
        Attribute attr = attributes[i];
        *(pair_location) = offset;
        *(pair_location + 4) = attr.len;
        pair_location += 8;
        *(start + offset) = attr;
        offset += attr.len;
    }
    *pair_location = null_array;
    Record new_record = {start};
    return &new_record;
}

