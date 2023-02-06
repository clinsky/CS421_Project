#include "query_processor.h"
#include "attribute_types.h"
#include "parse_utils.h"

// WILL PROBABLY NEED TO MOVE THIS TO A DIFFERENT FILE,
// SCAFFOLDING TO TEST QUICK
struct table {
  char *name;
  Attribute attributes[MAX_ATTRS];
};
typedef struct table Table;

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
  printf("%s is table_name\n", table_name);
  table_ptr->name = malloc(strlen(table_name) + 1);
  strcpy(table_ptr->name, table_name);
  while (1) {

    // first word
    scanf("%s", word);
    printf("here %s %lu\n", word, strlen(word));
    Attribute *attribute_ptr = malloc(sizeof(Attribute));
    attribute_ptr->name = malloc(strlen(word) + 1);

    // second word
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
    bool created_table = process_create_table();
  } else if (strcmp(word, "insert") == 0) {
    bool inserted_record = process_insert_record();
  } else if (strcmp(word, "select")) {
    bool valid_query = process_select();
  }
}

void query_loop() {
  while (1) {
    process();
  }
}
