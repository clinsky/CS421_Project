//
// Created by Jared Lantner on 2/11/23.
//

#include <stdio.h>
#include <stdlib.h>
#include "attribute_types.h"



#ifndef CS421_PROJECT_RECORD_H
#define CS421_PROJECT_RECORD_H

typedef struct pair Pair;
struct record {
    /*
     * Attributes represented by fixed size (offset, length) with actual data stored after all fixed
     * length attributes.
     */
    int * start;


};

typedef struct record Record;



#endif //CS421_PROJECT_RECORD_H