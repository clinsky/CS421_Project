#ifndef QUERY_PROCESSOR_H
#define QUERY_PROCESSOR_H

#include "query_processor.h"
#include "attribute_types.h"
#include "catalog.h"
#include "display.h"
#include "page.h"
#include "parse_utils.h"
#include "record.h"
#include "table.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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

bool parse_create_table(char *command, char *db_loc, Schema *schema) {
  bool has_primary_key = false;
  int command_len = strlen(command);
  command[command_len] = ';';
  command[command_len + 1] = '\0';
  char *token;

  // first token is "create", can skip this
  token = strtok(command, " ");

  // next token after create
  token = strtok(NULL, " ");

  // create keyword wasn't followed by "table"
  if (strcmp(token, "table") != 0) {
    return false;
  }

  char *table_name = strtok(NULL, " ");
  Table *table_ptr = malloc(sizeof(Table));
  if (!endsWith(table_name, "(")) {
    // should follow format create table foo(
    printf("invalid table name\n");
    return false;
  }
  table_ptr->name = malloc(strlen(table_name)); // no +1 because subtract the (
  strncpy(table_ptr->name, table_name, strlen(table_name) - 1);
  printf("%s (len %lu) is table name \n", table_ptr->name,
         strlen(table_ptr->name));

  // continue with rest of tokens
  while (1) {
    table_ptr->num_attributes++;
    Attribute *attribute_ptr = malloc(sizeof(Attribute));

    // field name
    token = strtok(NULL, " ");
    printf("token in loop: %s\n", token);
    //   printf("field name: %s (len %lu)\n", word, strlen(word));
    attribute_ptr->name = malloc(strlen(token) + 1);
    strncpy(attribute_ptr->name, token, strlen(token) + 1);

    printf("attr name: %s\n", token);

    // attr type
    token = strtok(NULL, " ");

    // last statement
    if (endsWith(token, ");")) {
      ATTRIBUTE_TYPE attr_type =
          parse_attribute_type_before(token, 2, attribute_ptr);
      if (attr_type == INVALID_ATTR) {
        return false;
      }
      table_ptr->attributes = (Attribute *)realloc(
          table_ptr->attributes, table_ptr->num_attributes * sizeof(Attribute));
      table_ptr->attributes[table_ptr->num_attributes - 1] = *attribute_ptr;
      break;
    }

    // ends with comma => more statements to follow
    if (endsWith(token, ",")) {
      ATTRIBUTE_TYPE attr_type =
          parse_attribute_type_before(token, 1, attribute_ptr);
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
        parse_attribute_type_before(token, 0, attribute_ptr);
    if (attr_type == INVALID_ATTR) {
      return false;
    }

    // 3RD WORD IF EXISTS SHOULD BE PRIMARYKEY
    token = strtok(NULL, " ");

    if (!startsWith(token, "primarykey")) {
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
    if (endsWith(token, ");")) {
      table_ptr->attributes = (Attribute *)realloc(
          table_ptr->attributes, table_ptr->num_attributes * sizeof(Attribute));
      table_ptr->attributes[table_ptr->num_attributes - 1] = *attribute_ptr;
      break;
    }

    // ends with comma => more statements to follow
    if (endsWith(token, ",")) {
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

  // UNCOMMENT THIS ONLY IF U WANT TO TEST AND SEE OUTPUT
  // print_table_metadata(table_ptr);
  // write_catalog(db_loc, table_ptr);
  // Schema *schema2 = read_catalog(db_loc);
  // for (int i = 0; i < schema2->num_tables; i++) {
  //   Table *curr_table = &schema2->tables[i];
  //   printf("table #%d name: %s\n", i, curr_table->name);
  //   for (int j = 0; j < curr_table->num_attributes; j++) {
  //     printf("attr #%d name: %s , type: %s , len: %d is_primary_key: %d\n",
  //     j,
  //            curr_table->attributes[j].name,
  //            attribute_type_to_string(curr_table->attributes[j].type),
  //            curr_table->attributes[j].len,
  //            curr_table->attributes[j].is_primary_key);
  //   }
  // }

  return true;
}

char **parse_tuple(char *tuple, Table *command_table) {
  printf("Type: %d\n",
         attribute_type_to_int(command_table->attributes[0].type));
  printf("Type: %d\n",
         attribute_type_to_int(command_table->attributes[1].type));

  // Remove parentheses from tuple
  sscanf(tuple, "(%[^)]", tuple);

  // Create array to be returned
  char tuple_parsed[command_table->num_attributes][50];

  // Iterate
  char current_token[50];
  char next_tokens[50];
  strcpy(next_tokens, tuple);
  for (int i = 0; i < command_table->num_attributes; i++) {
    if (command_table->attributes[i].type == CHAR ||
        command_table->attributes[i].type == VARCHAR) {
      if (next_tokens[0] == '"') {
        sscanf(next_tokens, "\"%[^\"]\" %[^\\0]", current_token, next_tokens);
      } else {
        sscanf(next_tokens, "%s %[^\\0]", current_token, next_tokens);
      }
    } else {
      sscanf(next_tokens, "%s %[^\\0]", current_token, next_tokens);
    }
    printf("current token: %s\n", current_token);
    printf("\n");
    printf("next tokens: %s\n", next_tokens);
    printf("\n");
  }
  printf("\n");
  return NULL;
}

bool process_insert_record(char *command, char *db_loc, Schema *schema) {
  // Add a semi-colon at the end of the command
  int command_len = strlen(command);
  command[command_len] = ';';
  command[command_len + 1] = '\0';

  // Parse the table name and values out of the command
  char table_name[20], values[50];
  sscanf(command, "insert into %s values %[^;]", table_name, values);

  // Make sure the values are comma delimited with no space
  char values_delimited[50];
  values_delimited[0] = values[0];
  char prev = values[0];
  int j = 1;
  for (int i = 1; i < strlen(values); i++) {
    if (!(prev == ',' && values[i] == ' ')) {
      values_delimited[j] = values[i];
      j++;
    }
    prev = values[i];
  }
  values_delimited[j] = '\0';

  // Print table name and values, sanity check
  printf("Table Name = %s\nValues = %s\n", table_name, values_delimited);

  // Count the number of values the user provided
  int num_values = 0;
  for (int i = 0; i < strlen(values_delimited); i++) {
    if (values_delimited[i] == ',') {
      num_values++;
    }
  }
  num_values++;

  // Create the array to be returned
  Table *command_table = get_table(schema, table_name);
  char values_parsed[num_values][command_table->num_attributes][50];

  // Iterate through values
  char *tuple = strtok(values_delimited, ",");
  while (tuple != NULL) {
    parse_tuple(tuple, command_table);
    tuple = strtok(NULL, ",");
  }
  return false;
}

void display_attributes(Schema *schema) {}

void display_record(Record record) {
  int **offset_len_pairs = record.offset_length_pairs;
  // TODO: Find a way to get the number of attributes
  int num_attributes = 3;
  for (int i = 0; i < num_attributes; i += 2) {
    int offset = offset_len_pairs[i][0];
    int len = offset_len_pairs[i][1];
    Attribute attr = record.attributes[offset];
    printf("%s ", attr.name);
  }
}

bool select_all(char *table_name, char *db_loc, Schema *schema) {
  Table *requested_table = NULL;
  Table *tables = schema->tables;
  int num_tables = schema->num_tables;
  printf("num_tables: %d\n", num_tables);
  for (int i = 0; i < num_tables; i++) {
    printf("name: %s\n", tables[i].name);
    if (strcmp(tables[i].name, table_name) == 0) {
      requested_table = &tables[i];
      break;
    }
  }
  if (!requested_table) {
    printf("No such table %s\n", table_name);
    return false;
  }

  // TODO: format the attributes as in the writeup
  int num_atrributes = requested_table->num_attributes;
  for (int i = 0; i < num_atrributes; i++) {
    Attribute attr = requested_table->attributes[i];
    printf("%s ", attr.name);
  }

  char *filename = strcat(db_loc, table_name);
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    // printf("No such table %s\n", table_name);
    return false;
  }

  // TODO: Use the design shown on the writeup

  display_attributes(schema);

  int *page_locations = schema->page_locations;
  int num_pages;
  Page page;
  fread(&num_pages, sizeof(int), 1, fp);
  for (int i = 0; i < num_pages; i++) {
    fseek(fp, page_locations[i], SEEK_SET);
    fread(&page, sizeof(Page), 1, fp);
    for (int j = 0; j < page.num_records; j++) {
      Record record = page.records[j];
      display_record(record);
    }
  }
  return true;
}

bool parse_select(char *command, char *db_loc, Schema *schema) {
  char attributes[256];
  char table_name[256];
  char *token = strtok(command, " ");
  token = strtok(NULL, " ");

  strcpy(attributes, token);

  token = strtok(NULL, " ");
  if (strcmp(token, "from") != 0) {
    printf("Syntax Error\n");
    return false;
  }

  token = strtok(NULL, " ");
  strcpy(table_name, token);
  printf("tableName: %s\n", table_name);
  if (strcmp(attributes, "*") == 0) {
    return select_all(table_name, db_loc, schema);
  }
  return true;
}

bool process_display_schema(char *command, char *db_loc, Schema *schema) {
  printf("Display Schema not implemented!");
  return false;
}

bool process_display_info(char *command, char *db_loc, Schema *schema) {
  // im pretty sure display_info is already implemented in parse_utils.c
  // but i havent looked at the write up in weeks so i dont remember what
  // display info is lol
  printf("Display Info not implemented!");
  return false;
}

void shut_down_database() { return; }

void purge_page_buffer() {}

void save_catalog(Schema *schema, char *db_loc) {
  char path[100];
  strcpy(path, db_loc);
  strcat(path, "/catalog");
  FILE *fp = fopen(path, "wb");
  fwrite(&(*schema), sizeof(Schema), 1, fp);
}

void parse_command(char *command, char *db_loc, Schema *schema) {
  // Self explanatory code.
  if (startsWith(command, "select")) {
    parse_select(command, db_loc, schema);
  } else if (startsWith(command, "create")) {
    parse_create_table(command, db_loc, schema);
  } else if (startsWith(command, "insert")) {
    process_insert_record(command, db_loc, schema);
  } else if (startsWith(command, "display schema")) {
    process_display_schema(command, db_loc, schema);
  } else if (startsWith(command, "display info")) {
    process_display_info(command, db_loc, schema);
  } else {
    printf("Invalid command\n");
  }
  printf("\n");
}

void process(char *db_loc, Schema *schema) {
  // Continuously accept commands from a user
  while (1) {
    // Iterate through characters typed by the user
    // Stop iterating if the user enters <quit> or a semi-colon
    printf(">");
    char command[500];
    command[0] = '\0';
    char next_char = '\0';
    int command_idx = 0;
    while (((next_char = getchar()) != ';')) {
      command[command_idx] = next_char;
      command_idx++;
      command[command_idx] = '\0';
      if (strcmp(command, "<quit>") == 0) {
        printf("Safely shutting down the database...\n");
        shut_down_database();
        printf("Purging page buffer...\n");
        purge_page_buffer();
        printf("Saving catalog...\n");
        save_catalog(schema, db_loc);
        printf("\n");
        printf("Exiting the database...\n");
        return;
      }
    }

    // Fill newlines with spaces
    while (strcspn(command, "\n") != strlen(command)) {
      command[strcspn(command, "\n")] = ' ';
    }

    // We've read a command from the user. Parse it and take some action
    parse_command(command, db_loc, schema);

    // I'm not sure what this does. Jared?
    next_char = getchar();
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

void query_loop(char *db_loc, Schema *schema) { process(db_loc, schema); }

#endif
