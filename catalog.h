#ifndef CATALOG_H
#define CATALOG_H

#include "bplus_tree.h"
#include "display.h"
#include "parse_utils.h"
#include "table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Schema
struct schema {
  char db_path[100];
  unsigned int page_size;
  unsigned int buffer_size;
  unsigned int num_tables;
  unsigned int max_num_tables;
  unsigned int *page_locations;
  Table *tables;
  BPlusTree *btrees;
};

typedef struct schema Schema;

// FORWARD DECLARE BUFFERM IN ORDER TO NOT INCLUDE PAGE.H
// THIS FIXES CYCLIC DEPENDENCY ISSUE
struct bufferm;

Schema *create_schema(char *db_loc, int page_size, int buffer_size);
void increment_table_count(char *db_loc);
void write_catalog(char *db_loc, Table *tables);
void add_table_to_catalog(Schema *db_schema, Table *table);
void create_catalog(char *db_loc);
Schema *read_catalog(char *db_loc);
Table *get_table(Schema *db_schema, char *table_name);
void write_schemas_to_catalog(Schema *db_schema);
bool alter_table_add(Schema *db_schema, struct bufferm *buffer,
                     char *table_name, Attribute *attr,
                     Attribute_Values *attr_val);
bool drop_table(Schema *db_schema, struct bufferm *buffer, char *table_name);
bool alter_table_drop(Schema *db_schema, struct bufferm *buffer,
                      char *table_name, char *attr_name);
Attribute_Values *clone_attr_vals(Attribute_Values *src);
#endif
