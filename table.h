#ifndef TABLE_H
#define TABLE_H
#include "attribute_types.h"

struct table {
  char *name;
  Attribute *attributes;
  int num_attributes;
  int page_count;
  int num_unique_attributes;
};
typedef struct table Table;

#endif
