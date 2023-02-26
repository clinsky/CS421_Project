#ifndef TABLE_H
#define TABLE_H
#include "attribute_types.h"

struct table {
  char *name;
  Attribute *attributes;
  int num_attributes;
  int * page_locations;
};

typedef struct table Table;

#endif
