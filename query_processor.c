#ifndef QUERY_PROCESSOR_H
#define QUERY_PROCESSOR_H

#include "query_processor.h"

#include "attribute_types.h"

#include "catalog.h"

#include "page.h"

#include "display.h"

#include "parse_utils.h"

#include "table.h"

#include <stdbool.h>

#include <stdio.h>

#include <string.h>

ATTRIBUTE_TYPE parse_attribute_type(char * attr, Attribute * attribute_ptr) {
  //  printf("trying to determine %s's type\n", attr);
  if (strcmp(attr, "integer") == 0) {
    attribute_ptr -> type = INTEGER;
    return INTEGER;
  } else if (strcmp(attr, "bool") == 0) {
    attribute_ptr -> type = BOOL;
    return BOOL;
  } else if (strcmp(attr, "double") == 0) {
    attribute_ptr -> type = DOUBLE;
    return DOUBLE;
  } else if (startsWith(attr,
      "char(")) { // need to check if of the form char(...
    int len;
    if (sscanf(attr, "char(%d)", & len) == 1) {
      printf("char len is %d\n", len);
      attribute_ptr -> type = CHAR;
      attribute_ptr -> len = len;
    }
    return CHAR;
  } else if (startsWith(
      attr,
      "varchar(")) { // need to check if of the form varchar(...
    int len;
    if (sscanf(attr, "varchar(%d)", & len) == 1) {
      attribute_ptr -> type = VARCHAR;
      attribute_ptr -> len = len;
    }
    return VARCHAR;
  }
  // parse failed
  printf("Invalid data type \"%s\"\n", attr);
  return INVALID_ATTR;
}

ATTRIBUTE_TYPE parse_attribute_type_before(char * attr,
  int num_last_char_exclude,
  Attribute * attribute_ptr) {
  char temp[strlen(attr) - num_last_char_exclude + 1];
  strncpy(temp, attr, strlen(attr) - num_last_char_exclude);
  return parse_attribute_type(temp, attribute_ptr);
}

/*
 * WORK IN PROGRESS
 *
 * */

bool parse_create_table(char * command, char * db_loc, Schema * schema, PageBuffer * page_buffer) {
  bool has_primary_key = false;
  int command_len = strlen(command);
  command[command_len] = ';';
  command[command_len + 1] = '\0';
  char * token;

  // first token is "create", can skip this
  token = strtok(command, " ");

  // next token after create
  token = strtok(NULL, " ");

  // create keyword wasn't followed by "table"
  if (strcmp(token, "table") != 0) {
    return false;
  }

  char * table_name = strtok(NULL, " ");

  Table * table_ptr = malloc(sizeof(Table));
  if (!endsWith(table_name, "(")) {
    // should follow format create table foo(
    printf("invalid table name\n");
    return false;
  }

  // remove the trailing (
  table_ptr -> name = malloc(strlen(table_name)); // no +1 because subtract the (
  strncpy(table_ptr -> name, table_name, strlen(table_name) - 1);
  table_ptr -> name[strlen(table_name) - 1] = '\0';

  // check no table in catalog with same name already
  Table * table_in_catalog = get_table(schema, table_ptr -> name);
  if (table_in_catalog != NULL) {
    printf("Table of name %s already exists\n", table_ptr -> name);
    return false;
  }

  table_ptr -> attributes = malloc(sizeof(Attribute) * 100);

  // printf("%s (len %lu) is table name \n", table_ptr->name,
  // strlen(table_ptr->name));

  // continue with rest of tokens
  table_ptr -> num_attributes = 0;
  while (1) {
    table_ptr -> num_attributes++;
    Attribute * attribute_ptr = malloc(sizeof(Attribute));

    // field name
    token = strtok(NULL, " ");
    //   printf("field name: %s (len %lu)\n", word, strlen(word));
    attribute_ptr -> name = malloc(strlen(token) + 1);
    strncpy(attribute_ptr -> name, token, strlen(token) + 1);

    // attr type
    token = strtok(NULL, " ");

    // last statement
    if (endsWith(token, ");")) {
      ATTRIBUTE_TYPE attr_type =
        parse_attribute_type_before(token, 2, attribute_ptr);
      if (attr_type == INVALID_ATTR) {
        return false;
      }
      table_ptr -> attributes[table_ptr -> num_attributes - 1] = * attribute_ptr;
      break;
    }

    // ends with comma => more statements to follow
    if (endsWith(token, ",")) {
      ATTRIBUTE_TYPE attr_type =
        parse_attribute_type_before(token, 1, attribute_ptr);
      if (attr_type == INVALID_ATTR) {
        return false;
      }
      table_ptr -> attributes[table_ptr -> num_attributes - 1] = * attribute_ptr;
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
    attribute_ptr -> is_primary_key = true;

    // last statement of the form num integer primarykey);
    if (endsWith(token, ");")) {
      table_ptr -> attributes[table_ptr -> num_attributes - 1] = * attribute_ptr;
      break;
    }

    // ends with comma => more statements to follow
    if (endsWith(token, ",")) {
      table_ptr -> attributes[table_ptr -> num_attributes - 1] = * attribute_ptr;
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

  write_catalog(db_loc, table_ptr);

  // need to also update schema
  schema -> num_tables += 1;
  schema -> tables[schema -> num_tables - 1] = * table_ptr;

  Page * new_page = (Page * ) malloc(sizeof(Page));
  new_page -> records = NULL;
  new_page -> num_records = 0;
  new_page -> num_bytes = 0;
  new_page -> table_name = table_ptr -> name;
  new_page -> page_index = 0;

  int page_index = get_page_index(page_buffer, schema);
  page_buffer -> pages[page_index] = * new_page;
  page_buffer -> last_used_count++;
  page_buffer -> last_used[page_index] = page_buffer -> last_used_count;

  return true;
}

bool parse_tuple(char * tuple, char ** tuple_parsed, Table * command_table) {
  // Remove parentheses from tuple
  sscanf(tuple, "(%[^)]", tuple);

  // Iterate
  char current_token[50];
  char next_tokens[256];
  strcpy(next_tokens, tuple);
  for (int i = 0; i < command_table -> num_attributes; i++) {
    bool null = false;
    if (command_table -> attributes[i].type == CHAR ||
      command_table -> attributes[i].type == VARCHAR) {
      if (next_tokens[0] == '"') {
        sscanf(next_tokens, "\"%[^\"]\" %[^\\0]", current_token, next_tokens);
        if (strlen(current_token) > command_table -> attributes[i].len) {
          printf("Invalid data type, char is too long!");
          return false;
        }
      } else {
        null = true;
        sscanf(next_tokens, "%s %[^\\0]", current_token, next_tokens);
        if (strcmp(current_token, "null") != 0) {
          printf("Invalid data type, chars must be in quotes!");
          return false;
        }
      }
    } else {
      sscanf(next_tokens, "%s %[^\\0]", current_token, next_tokens);
      if (strcmp(current_token, "null") == 0) {
        null = true;
      }
    }
    if (!null) {
      tuple_parsed[i] = (char * ) malloc((strlen(current_token) + 1) * sizeof(char));
      strcpy(tuple_parsed[i], current_token);
    }
  }
  return true;
}

bool process_insert_record(char * command, char * db_loc, Schema * schema, PageBuffer * page_buffer) {
  // Add a semi-colon at the end of the command
  int command_len = strlen(command);
  command[command_len] = ';';
  command[command_len + 1] = '\0';

  // Parse the table name and values out of the command
  char table_name[50], values[256];
  sscanf(command, "insert into %s values %[^;]", table_name, values);

  // Get the table
  Table * command_table = get_table(schema, table_name);

  // Make sure the values are comma delimited with no space
  char values_delimited[256];
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

  // Iterate through values
  char * tuple = strtok(values_delimited, ",");
  int tuple_index = 0;
  while (tuple != NULL) {
    char * tuple_parsed[command_table -> num_attributes];
    if (!parse_tuple(tuple, tuple_parsed, command_table)) {
      return false;
    }
    printf("%s %s\n", tuple_parsed[0], tuple_parsed[1]);
    tuple = strtok(NULL, ",");
    tuple_index++;
  }
  return true;
}

bool parse_select(char * command, char * db_loc, Schema * schema) {
  printf("Parse select not implemented!");
  return false;
}

bool process_display_schema(char * command, char * db_loc, Schema * schema) {
  printf("Display Schema not implemented!");
  return false;
}

bool process_display_info(char * command, char * db_loc, Schema * schema) {
  char table_name[50];
  sscanf(command, "display info %s", & table_name);
  Table * table = get_table(schema, & table_name);
  if (table == NULL) {
    printf("table cant be displayed, was null\n");
  } else {
    print_table_metadata(table);
  }

  return false;
}

void shut_down_database() {
  return;
}

void purge_page_buffer() {}

void save_catalog(Schema * schema, char * db_loc) {
  char path[100];
  strcpy(path, db_loc);
  strcat(path, "/catalog");
  FILE * fp = fopen(path, "wb");
  fwrite( & ( * schema), sizeof(Schema), 1, fp);
}

void parse_command(char * command, char * db_loc, Schema * schema, struct page_buffer * page_buffer) {
  // Self explanatory code.
  if (startsWith(command, "select")) {
    parse_select(command, db_loc, schema);
  } else if (startsWith(command, "create")) {
    parse_create_table(command, db_loc, schema, page_buffer);
  } else if (startsWith(command, "insert")) {
    process_insert_record(command, db_loc, schema, page_buffer);
  } else if (startsWith(command, "display schema")) {
    process_display_schema(command, db_loc, schema);
  } else if (startsWith(command, "display info")) {
    process_display_info(command, db_loc, schema);
  } else {
    printf("Invalid command\n");
  }
  printf("\n");
}

void process(char * db_loc, Schema * schema, PageBuffer * page_buffer) {
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
    parse_command(command, db_loc, schema, page_buffer);

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

#endif