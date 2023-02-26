#ifndef RECORD_H
#define RECORD_H


#include <stdio.h>
#include <stdlib.h>
#include "attribute_types.h"
#include "table.h"

struct record {
    int * offsets;
    int * lengths;
    char * null_array;
    char ** data;
};

typedef struct record Record;



#endif
