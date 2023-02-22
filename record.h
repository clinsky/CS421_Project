#ifndef RECORD_H
#define RECORD_H


#include <stdio.h>
#include <stdlib.h>
#include "attribute_types.h"
#include "table.h"

struct record {
    int * offset_length_pairs[2];
    Attribute * attributes;
};

typedef struct record Record;



#endif
