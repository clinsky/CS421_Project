//
// Created by Jared Lantner on 2/11/23.
//

#include "record.h"

Record * init_record(int num_fields) {
    /**
     * Record should be variable length
     */
    Record *new_record = (Record *) malloc(8);
    new_record->fields = (Attribute *) malloc(sizeof(Attribute) * num_fields);
    new_record->num_attributes = num_fields;
    return new_record;
}

