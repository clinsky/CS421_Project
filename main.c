#include "bplus_tree.h"
#include "catalog.h"
#include "page.h"
#include "parse_utils.h"
#include "query_processor.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

// check if a directory exists
bool directory_exists(char *dir) {
  struct stat st = {0};
  if (stat(dir, &st) == -1) {
    return false;
  }
  return true;
}

bool database_exists(char *db_loc) {

  struct stat str = {0};
  if (stat(db_loc, &str) == -1) {
    return false;
  }
  return true;
  /*
  FILE * db_file = fopen(db_loc, "rb");
  if (db_file) {
      return true;
  }
  return false;
   */
}

Schema *create_new_database(char *db_loc, int page_size, int buffer_size) {
  mkdir(db_loc, 0755);
  create_catalog(db_loc);
  Schema *new_schema = create_schema(db_loc, page_size, buffer_size);

  char path[100];

  strcpy(path, db_loc);
  strcat(path, "/tables");
  mkdir(path, 0755);
  return new_schema;
}

Schema *restart_database(char *db_loc, int page_size, int buffer_size) {
  // read_catalog reconstructs tables stored in catalog into schema
  Schema *schema = read_catalog(db_loc);

  // still need to set these fields still
  strncpy(schema->db_path, db_loc, strlen(db_loc));
  schema->page_size = page_size;
  schema->buffer_size = buffer_size;
  schema->max_num_tables = 10;
  return schema;
}

int main(int argc, char *argv[]) {
  printf("Welcome to Group8QL\n");
  char *db_loc = argv[1];
  int page_size = atoi(argv[2]);
  int buffer_size = atoi(argv[3]);
  char *indexing = argv[4];
  Schema *schema;
  printf("Looking at %s for existing db...\n", db_loc);
  if (database_exists(db_loc)) {
    printf("Database found...\n");
    printf("Restarting the database...\n");
    schema = restart_database(db_loc, page_size, buffer_size);
  } else {
    printf("No existing db found\n");
    printf("Creating new db at %s\n", db_loc);
    printf("New db created succesfully\n");
    schema = create_new_database(db_loc, page_size, buffer_size);
  }
  printf("Page size: %d\n", page_size);
  printf("Buffer size: %d\n", buffer_size);
  Bufferm *bufferm = create_new_bufferm(buffer_size);

  if (strcmp(indexing, "true") == 0) {
    schema->indexing = true;
    schema->btrees = malloc(sizeof(BPlusTree) * schema->num_tables);
    printf("\nCreating indexes...\n");
    for (int i = 0; i < schema->num_tables; i++) {
      Table table = schema->tables[i];
      Attribute attr;
      for (int j = 0; j < table.num_attributes; j++) {
        if (table.attributes[j].is_primary_key) {
          attr = table.attributes[j];
        }
      }
      printf("Creating index for attribute '%s' in table '%s'\n", attr.name,
             table.name);
      int max_size = 4;
      if (attr.type == INTEGER) {
        max_size += 4;
      } else if (attr.type == DOUBLE) {
        max_size += 8;
      } else if (attr.type == BOOL) {
        max_size += 4;
      } else if (attr.type == CHAR) {
        max_size += attr.len;
      } else if (attr.type == VARCHAR) {
        max_size += attr.len;
      }
      int b_tree_n = page_size / max_size - 1;
      BPlusTree *index = init_BPlusTree(b_tree_n, true, false);
      schema->btrees[i] = *index;
    }
  } else {
    schema->indexing = false;
  }

  printf("\nPlease enter commands, enter <quit> to shutdown the db\n");
  process(db_loc, schema, bufferm);
  return 0;
}
