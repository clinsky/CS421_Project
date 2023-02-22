#include "page.h"
#include "catalog.h"
#include "parse_utils.h"
#include "query_processor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool database_exists(char * db_loc){
    FILE * db_file = fopen(db_loc, "rb");
    if (db_file) {
        return true;
    }
    return false;
}

void create_new_database(char * db_loc, int page_size, int buffer_size){
   Schema new_schema =  create_schema(db_loc, page_size, buffer_size);
   // TODO: create a table directory in db_loc
}

void read_in_catalog(){

}

void restart_database(int buffer_size){
    read_in_catalog();

}

void process_command(char * command){
    process(command);
}

int main(int argc, char *argv[]) {
  char *db_loc = argv[1];
  int page_size = atoi(argv[2]);
  int buffer_size = atoi(argv[3]);

  if(database_exists(db_loc)){
      restart_database(buffer_size);
  }
  else{
      create_new_database(db_loc, page_size, buffer_size);
  }
    char * command;
    printf(">");
    scanf("%s", command);
    while(strcmp(command, "quit") != -1){
      process_command(command);
      printf("\n>");
      scanf("%s", command);
  }

  printf("db_loc: %s\npage_size: %d\nbuffer_size:%d\n", db_loc, page_size,
         buffer_size);

  return 0;
}



