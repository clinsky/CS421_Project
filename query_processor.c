#ifndef QUERY_PROCESSOR_H
#define QUERY_PROCESSOR_H

#include "query_processor.h"
#include "attribute_types.h"
#include "parse_utils.h"
#include "table.h"
#include "catalog.h"
#include "page.h"
#include "record.h"
#include "display.h"



ATTRIBUTE_TYPE parse_attribute_type(char *attr, Attribute *attribute_ptr) {
  //  printf("trying to determine %s's type\n", attr);
  if (strcmp(attr, "integer") == 0) {
    attribute_ptr->type = INTEGER;
    return INTEGER;
  } else if (strcmp(attr, "bool") == 0) {
    attribute_ptr->type = BOOL;
    return BOOL;
  } else if (strcmp(attr, "double") == 0) {
    attribute_ptr->type = DOUBLE;
    return DOUBLE;
  } else if (startsWith(attr,
                        "char(")) { // need to check if of the form char(...
    int len;
    if (sscanf(attr, "char(%d)", &len) == 1) {
      printf("char len is %d\n", len);
      attribute_ptr->type = CHAR;
      attribute_ptr->len = len;
    }
    return CHAR;
  } else if (startsWith(
                 attr,
                 "varchar(")) { // need to check if of the form varchar(...
    int len;
    if (sscanf(attr, "varchar(%d)", &len) == 1) {
      attribute_ptr->type = VARCHAR;
      attribute_ptr->len = len;
    }
    return VARCHAR;
  }
  // parse failed
  printf("Invalid data type \"%s\"\n", attr);
  return INVALID_ATTR;
}

ATTRIBUTE_TYPE parse_attribute_type_before(char *attr,
                                           int num_last_char_exclude,
                                           Attribute *attribute_ptr) {
    char temp[strlen(attr) - num_last_char_exclude + 1];
    strncpy(temp, attr, strlen(attr) - num_last_char_exclude);
    return parse_attribute_type(temp, attribute_ptr);
}

/*
 * WORK IN PROGRESS
 *
 * */


bool process_create_table() {
  int MAX_NAME_LEN = 100;
  int MAX_ATTR_LEN = 100;
  char word[MAX_NAME_LEN];
  char table_name[MAX_NAME_LEN];
  bool has_primary_key = false;
  scanf("%s", word);

  // create keyword wasn't followed by "table"
  if (strcmp(word, "table") != 0) {
    return false;
  }

  Table *table_ptr = malloc(sizeof(Table));
  scanf("%s", table_name);
  if (!endsWith(table_name, "(")) {
    // should follow format create table foo(
    printf("invalid table name\n");
    return false;
  }
  table_ptr->name = malloc(strlen(table_name)); // no +1 because subtract the (
  strncpy(table_ptr->name, table_name, strlen(table_name) - 1);
  //  printf("%s (len %lu) is table name \n", table_ptr->name,
  // strlen(table_ptr->name));

  while (1) {
    table_ptr->num_attributes++;
    Attribute *attribute_ptr = malloc(sizeof(Attribute));

    // field name
    scanf("%s", word);
    //   printf("field name: %s (len %lu)\n", word, strlen(word));
    attribute_ptr->name = malloc(strlen(word) + 1);
    strncpy(attribute_ptr->name, word, strlen(word) + 1);

    // attr type
    scanf("%s", word);

    // last statement
    if (endsWith(word, ");")) {
      ATTRIBUTE_TYPE attr_type =
          parse_attribute_type_before(word, 2, attribute_ptr);
      if (attr_type == INVALID_ATTR) {
        return false;
      }
      table_ptr->attributes = (Attribute *)realloc(
          table_ptr->attributes, table_ptr->num_attributes * sizeof(Attribute));
      table_ptr->attributes[table_ptr->num_attributes - 1] = *attribute_ptr;
      break;
    }

    // ends with comma => more statements to follow
    if (endsWith(word, ",")) {
      ATTRIBUTE_TYPE attr_type =
          parse_attribute_type_before(word, 1, attribute_ptr);
      if (attr_type == INVALID_ATTR) {
        return false;
      }
      table_ptr->attributes = (Attribute *)realloc(
          table_ptr->attributes, table_ptr->num_attributes * sizeof(Attribute));
      table_ptr->attributes[table_ptr->num_attributes - 1] = *attribute_ptr;
      continue;
    }

    // of the form " ... x int primarykey... "
    ATTRIBUTE_TYPE attr_type =
        parse_attribute_type_before(word, 0, attribute_ptr);
    if (attr_type == INVALID_ATTR) {
      return false;
    }

    // 3RD WORD IF EXISTS SHOULD BE PRIMARYKEY
    scanf("%s", word);

    if (!startsWith(word, "primarykey")) {
      return false;
    }

    // check if already have primarykey
    if (has_primary_key) {
      printf("multiple primarykeys\n");
      return false;
    }

    has_primary_key = true;
    attribute_ptr->is_primary_key = true;

    // last statement of the form num integer primarykey);
    if (endsWith(word, ");")) {
      table_ptr->attributes = (Attribute *)realloc(
          table_ptr->attributes, table_ptr->num_attributes * sizeof(Attribute));
      table_ptr->attributes[table_ptr->num_attributes - 1] = *attribute_ptr;
      break;
    }

    // ends with comma => more statements to follow
    if (endsWith(word, ",")) {
      table_ptr->attributes = (Attribute *)realloc(
          table_ptr->attributes, table_ptr->num_attributes * sizeof(Attribute));
      table_ptr->attributes[table_ptr->num_attributes - 1] = *attribute_ptr;
      continue;
    }
    // shouldn't have anyother options i think...
    return false;
  }

  // check has primary key
  if (!has_primary_key) {
    printf("No primary key defined\n");
    return false;
  }

  print_table_metadata(table_ptr);
  return true;
}

// unimplemented at the moment
bool process_insert_record() { return false; }

void display_attributes(Schema schema){

}

void display_record(Record record){
    int ** offset_len_pairs = record.offset_length_pairs;
    //TODO: Find a way to get the number of attributes
    int num_attributes = 3;
    for(int i = 0; i < num_attributes; i+=2){
        int offset = offset_len_pairs[i][0];
        int len = offset_len_pairs[i][1];
        Attribute attr = record.attributes[offset];
        printf("%s ", attr.name);
    }
}

bool select_all(char *table_name, char *db_loc, Schema schema) {
    char * filename = strcat(db_loc, table_name);
    FILE *fp = fopen(filename, "r");
    if(fp == NULL) {
        printf("No such table %s\n", table_name);
        return false;
    }

    // TODO: Display the attributes of the table
    // TODO: Use the design shown on the writeup

    display_attributes(schema);

    int * page_locations = schema.page_locations;
    int num_pages;
    Page page;
    fread(&num_pages, sizeof(int), 1, fp);
    for(int i = 0; i < num_pages; i++) {
        fseek(fp, page_locations[i], SEEK_SET);
        fread(&page, sizeof(Page), 1, fp);
        for(int j = 0; j < page.num_records; j++) {
            Record record = page.records[j];
            display_record(record);
        }
    }
    return true;

}





bool process_select(char * command, char * db_loc, Schema schema) {
    char attributes[256];
    char table_name[256];
    char *token = strtok(command, " ");
    token = strtok(NULL, " ");

    strcpy(attributes, token);

    token = strtok(NULL, " ");
    if(strcmp(token, "from") != 0){
        printf("Syntax Error\n");
        return false;
    }

    token = strtok(NULL, " ");
    strcpy(table_name, token);
    if(strcmp(attributes, "*") == 0) {
        return select_all(table_name, db_loc, schema);
    }
    return true;
}


bool process_display_schema(char * command, char * db_loc, Schema schema){
    return true;
}

bool process_display_info(char * command, char * db_loc, Schema schema){
    return true;
}

void parse_command(char *command, char * db_loc, Schema schema){

    if(startsWith(command, "select") == true){
        process_select(command, db_loc, schema);
    }
    else if(startsWith(command, "insert") == true){
        process_insert_record(command, db_loc, schema);
    }
    else if(startsWith(command, "display schema") == true){
        process_display_schema(command, db_loc, schema);
    }
    else if(startsWith(command, "display info") == true){
        process_display_info(command, db_loc, schema);
    }
    else{
        printf("Invalid command\n");
    }
}


void process(char * db_loc, Schema schema){
  // TODO: SELECT * FROM <table_name>
  // TODO: INSERT
  // TODO: DISPLAY SCHEMA
  // TODO: DISPLAY INFO
  char command[256];
  printf(">");
  fgets(command, 256, stdin);
  while (strcmp(command, "quit\n") != 0) {

    parse_command(command, db_loc, schema);
    /*
    if (strcmp(word, "create") == 0) {
      print_command_result(process_create_table());
    } else if (strcmp(word, "insert") == 0) {
      print_command_result(process_insert_record());
    } else if (strcmp(word, "select") == 0) {
      print_command_result(process_select());
    } else if (strcmp(word, "q") == 0 || strcmp(word, "<quit>") == 0) {
      break;
    } else {
      printf("INVALID QUERY\n");
    }
     */
    printf(">");
    fgets(command, 256, stdin);
  }
}

void print_command_result(bool success) {
  if (success) {
    printf("SUCCESS\n");
  } else {
    fflush(stdin);
    printf("ERROR\n");
  }
}

void query_loop(char * db_loc, Schema schema) { process(db_loc, schema); }

#endif