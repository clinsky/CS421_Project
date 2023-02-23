#include "page.h"
#include "catalog.h"
#include "parse_utils.h"
#include "query_processor.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>


// check if a directory exists
bool directory_exists(char * dir){
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        return false;
    }
    return true;
}

bool database_exists(char * db_loc){

    struct stat str = {0};
    printf("%d\n", stat(db_loc, &str));
    if(stat(db_loc, &str) == -1) {
        printf("false\n");
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

Schema * create_new_database(char * db_loc, int page_size, int buffer_size){
    printf("Jared\n");
   Schema * new_schema =  create_schema(db_loc, page_size, buffer_size);
   printf("num tables: %d\n", new_schema->num_tables);
   printf("Lantner\n");


   mkdir(db_loc, 0755);
   char path[100];

   strcpy(path, db_loc);
   strcat(path, "/tables");
   mkdir(path, 0755);
   printf("num tables: %d\n", new_schema->num_tables);
   return new_schema;
}

Schema read_in_catalog(char * db_loc){
    Schema schema;
    char path[100];
    strcpy(path, db_loc);
    strcat(path, "/catalog");
    FILE *fp = fopen(path, "rb");
    fread(&schema, sizeof(Schema), 1, fp);
    return schema;
}

Schema * restart_database(char * db_loc, int buffer_size){
    Schema schema = read_in_catalog(db_loc);
    Schema * new_schema = malloc(sizeof(Schema));
    *new_schema = schema;
    return new_schema;
}


int main(int argc, char *argv[]) {
  char *db_loc = argv[1];
  int page_size = atoi(argv[2]);
  int buffer_size = atoi(argv[3]);
  Schema * schema;
  printf("%s\n", db_loc);
  if(database_exists(db_loc)){
      schema = restart_database(db_loc, buffer_size);
  }
  else{
      printf("new\n");
      schema = create_new_database(db_loc, page_size, buffer_size);
      printf("num tables: %d\n", schema->num_tables);
  }

    process(db_loc, schema);
  return 0;
}



