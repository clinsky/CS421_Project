#ifndef RECORD_H
#define RECORD_H


#include <stdio.h>
#include <stdlib.h>
#include "attribute_types.h"
#include "table.h"
#include "catalog.h"

struct record {
    int * offsets;
    int * lengths;
    char * null_array;
    char ** data;
};

typedef struct record Record;

Record create_record(int num_fields, char ** values, Schema * schema, int table_idx);
int record_size(Record record, Schema * schema, int table_idx);
#endif
