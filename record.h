#ifndef RECORD_H
#define RECORD_H

#include "attribute_types.h"
#include "conditional_parser.h"
#include "table.h"
#include <stdio.h>
#include <stdlib.h>

struct record {
    Attribute_Values *attr_vals;
    int bitmap;
    int size;
    int primary_key_index;
    int *unique_attribute_indices;
};

typedef struct record Record;

bool evaluateCondition (Record * record, ConditionalParseTree * conditionalParseTree, Table * table);

#endif
