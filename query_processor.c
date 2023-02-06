#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// defaults for now
#define MAX_NAME_LEN 100
#define MAX_ATTR_LEN 100
#define MAX_ATTRS 100

struct attribute {
  char name[MAX_NAME_LEN];
  char type[MAX_ATTR_LEN];
};

typedef struct attribute Attribute;

struct table {
  char *name;
  Attribute attributes[MAX_ATTRS];
};

typedef struct table Table;

void query_loop() {}

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
  table_ptr->name = table_name;
  while (1) {
    scanf("%s", word);
    printf("here %s %lu\n", word, strlen(word));
    strcpy(attributes[num_attributes].name, word);
    scanf("%s", word);
    strcpy(attributes[num_attributes].type, word);
    printf("%s is attr %s is type\n", attributes[num_attributes].name,
           attributes[num_attributes].type);

    char last_char = attributes[num_attributes]
                         .type[strlen(attributes[num_attributes].type) - 1];

    if (last_char == ',') {
      continue;
    }
    if (last_char == ')') {
      break;
    }
    printf("%lu is len\n", strlen(attributes[num_attributes].type));
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

bool process_insert_record() { return false; }

void process() {
  char word[100];
  scanf("%s", word);
  if (strcmp(word, "create") == 0) {
    bool created_table = process_create_table();
  } else if (strcmp(word, "insert") == 0) {
    bool inserted_record = process_insert_record();
  } else if (strcmp(word, "select")) {
  }
}
