#ifndef CATALOG_H
#define CATALOG_H

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
};

typedef struct schema Schema;

Schema *create_schema(char *db_loc, int page_size, int buffer_size);
void increment_table_count(char *db_loc);
void write_catalog(char *db_loc, Table *tables);
void create_catalog(char *db_loc);
Schema *read_catalog(char *db_loc);
Table *get_table(Schema *db_schema, char *table_name);
void TESTCATALOG();

#endif
