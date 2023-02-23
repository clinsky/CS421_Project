#include "page.h"
#include "catalog.h"
#include "parse_utils.h"
#include "query_processor.h"
#include <stdio.h>
#include <stdlib.h>

bool database_exists(char * db_loc){
    FILE * db_file = fopen(db_loc, "rb");
    if (db_file) {
        return true;
    }
    return false;
}

Schema create_new_database(char * db_loc, int page_size, int buffer_size){
   Schema new_schema =  create_schema(db_loc, page_size, buffer_size);
   // TODO: create a table directory in db_loc
   return new_schema;
}

void read_in_catalog(){

}

Schema restart_database(int buffer_size){
    read_in_catalog();

}


int main(int argc, char *argv[]) {
  char *db_loc = argv[1];
  int page_size = atoi(argv[2]);
  int buffer_size = atoi(argv[3]);
  Schema schema;
  if(database_exists(db_loc)){
      schema = restart_database(buffer_size);
  }
  else{
      schema = create_new_database(db_loc, page_size, buffer_size);
  }

    process(db_loc, &schema);
  return 0;
}



