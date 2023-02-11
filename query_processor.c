#include "query_processor.h"
#include "attribute_types.h"
#include "parse_utils.h"
#include "table.h"

/*
 * WORK IN PROGRESS
 *
 * */
bool process_create_table() {
  char word[MAX_NAME_LEN];
  char table_name[MAX_NAME_LEN];
  Attribute attributes[MAX_ATTRS];
  int num_attributes = 0;
  int primary_key_index = -1;
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
  printf("%s (len %lu) is table name \n", table_ptr->name,
         strlen(table_ptr->name));

  while (1) {
    // field name
    scanf("%s", word);
    printf("here %s %lu\n", word, strlen(word));
    Attribute *attribute_ptr = malloc(sizeof(Attribute));
    attribute_ptr->name = malloc(strlen(word) + 1);

    // attr type
    scanf("%s", word);
    // strcpy(attributes[num_attributes].type, word);
    // printf("%s is attr %s is type\n", attributes[num_attributes].name,
    //        attributes[num_attributes].type);

    // need to check last char to see if its a commma
    char last_char = word[strlen(word) - 1];
    endsWith(word, ")");

    // case where its something like 'num integer,'
    if (last_char == ',') {
      continue;
    }
    if (last_char == ')') {
      break;
    }
    scanf("%s", word);
    if (strcmp(word, ")") == 0) {
      break;
    }
    if (strcmp(word, "primarykey") == 0) {
      primary_key_index = num_attributes;
      printf("%s is primary key", attributes[num_attributes].name);
    }
    num_attributes++;
  }
  return false;
}

// unimplemented at the moment
bool process_insert_record() { return false; }

// unimplemented at the moment
bool process_select() { return false; }

void process() {
  char word[100];
  scanf("%s", word);
  if (strcmp(word, "create") == 0) {
    printf("attempting to parse create statement..\n");
    print_command_result(process_create_table());
  } else if (strcmp(word, "insert") == 0) {
    printf("attempting to parse insert statement..\n");
    print_command_result(process_insert_record());
  } else if (strcmp(word, "select")) {
    print_command_result(process_select());
    printf("attempting to parse select statement..\n");
  }
}

void print_command_result(bool success) {
  if (success) {
    printf("SUCCESS\n");
  } else {
    printf("ERROR\n");
  }
}

void query_loop() {
  while (1) {
    process();
  }
}
