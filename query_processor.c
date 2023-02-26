#ifndef QUERY_PROCESSOR_H
#define QUERY_PROCESSOR_H

#include "query_processor.h"
#include "attribute_types.h"
#include "buffer.h"
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
  table_ptr->attributes = malloc(sizeof(Attribute) * 100);
  if (!endsWith(table_name, "(")) {
    // should follow format create table foo(
    printf("invalid table name\n");
    return false;
  }

  // remove the trailing (
  table_ptr->name = malloc(strlen(table_name)); // no +1 because subtract the (
  strncpy(table_ptr->name, table_name, strlen(table_name) - 1);
  table_ptr->name[strlen(table_name) - 1] = '\0';

  // check no table in catalog with same name already
  Table *table_in_catalog = get_table(schema, table_ptr->name);
  if (table_in_catalog != NULL) {
    printf("Table of name %s already exists\n", table_ptr->name);
    return false;
  }

  table_ptr->attributes = malloc(sizeof(Attribute) * 100);

  // printf("%s (len %lu) is table name \n", table_ptr->name,
  // strlen(table_ptr->name));

  // continue with rest of tokens
  table_ptr->num_attributes = 0;
  while (1) {
    table_ptr->num_attributes++;
    Attribute *attribute_ptr = malloc(sizeof(Attribute));

    // field name
    token = strtok(NULL, " ");
    //   printf("field name: %s (len %lu)\n", word, strlen(word));
    attribute_ptr->name = malloc(strlen(token) + 1);
    strncpy(attribute_ptr->name, token, strlen(token) + 1);

    // attr type
    token = strtok(NULL, " ");

    // last statement
    if (endsWith(token, ");")) {
      ATTRIBUTE_TYPE attr_type =
          parse_attribute_type_before(token, 2, attribute_ptr);
      if (attr_type == INVALID_ATTR) {
        return false;
      }
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
      table_ptr->attributes[table_ptr->num_attributes - 1] = *attribute_ptr;
      break;
    }

    // ends with comma => more statements to follow
    if (endsWith(token, ",")) {
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

  write_catalog(db_loc, table_ptr);

  // need to also update schema
  schema->num_tables += 1;
  schema->tables[schema->num_tables - 1] = *table_ptr;

  // UNCOMMENT THIS ONLY IF U WANT TO TEST AND SEE OUTPUT
  printf("the table that was just created: ..\n");
  Schema *schema2 = read_catalog(db_loc);
  for (int i = 0; i < schema2->num_tables; i++) {
    Table *curr_table = &schema2->tables[i];
    printf("table #%d name: %s\n", i, curr_table->name);
    for (int j = 0; j < curr_table->num_attributes; j++) {
      printf("attr #%d name: %s , type: %s , len: %d is_primary_key: %d\n", j,
             curr_table->attributes[j].name,
             attribute_type_to_string(curr_table->attributes[j].type),
             curr_table->attributes[j].len,
             curr_table->attributes[j].is_primary_key);
    }
  }

  return true;
}

bool parse_tuple(char *tuple, char ***values_parsed, int tuple_index, Table *command_table) {
  // Remove parentheses from tuple
  sscanf(tuple, "(%[^)]", tuple);
  tuple[strlen(tuple)] = '\n';

  // Iterate
  char current_token[50];
  char next_tokens[256];
  strcpy(next_tokens, tuple);
  for (int i = 0; i < command_table->num_attributes; i++) {
    bool null = false;
    if (command_table->attributes[i].type == CHAR ||
        command_table->attributes[i].type == VARCHAR) {
      if (next_tokens[0] == '"') {
        sscanf(next_tokens, "\"%[^\"]\" %[^\n]", current_token, next_tokens);
        if (strlen(current_token) > command_table->attributes[i].len) {
          printf("Invalid data type, char is too long!");
          return false;
        }
      } else {
        sscanf(next_tokens, "%s %[^\n]", current_token, next_tokens);
        null = true;
        if (strcmp(current_token, "null") != 0) {
          printf("Invalid data type, chars must be in quotes!");
          return false;
        }
      }
    } else {
      sscanf(next_tokens, "%s %[^\n]", current_token, next_tokens);
      if (strcmp(current_token, "null") == 0) {
        null = true;
      }
    }
    if (!null) {
      values_parsed[tuple_index][i] = (char * ) malloc((strlen(current_token) + 1) * sizeof(char));
      strcpy(values_parsed[tuple_index][i], current_token);
    }
    else{
      values_parsed[tuple_index][i] = NULL;
    }
  }
  return true;
}

bool process_insert_record(char *command, char *db_loc, Schema *schema,
                           Bufferm *buffer) {
  // Add a semi-colon at the end of the command
  int command_len = strlen(command);
  command[command_len] = ';';
  command[command_len + 1] = '\0';

  // Parse the table name and values out of the command
  char table_name[50], values[256];
  sscanf(command, "insert into %s values %[^;]", table_name, values);

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

  // Create the array to be returned
  Table *command_table = get_table(schema, table_name);
  char ***values_parsed;
  values_parsed = (char ***)malloc(num_values * sizeof(char **));
  for (int i = 0; i < command_table->num_attributes; i++) {
    values_parsed[i] = (char **)malloc(command_table->num_attributes * sizeof(char *));
  }

  // Iterate through values
  char *tuple = strtok(values_delimited, ",");
  int tuple_index = 0;
  while (tuple != NULL) {
    if (!parse_tuple(tuple, values_parsed, tuple_index, command_table)) {
      return false;
    }
    tuple = strtok(NULL, ",");
    tuple_index++;
  }
  printf("num values: %d\n", num_values);
  printf("printing values parsed\n");

  Record *records = malloc(sizeof(Record) * num_values);

  // validate
  for (int i = 0; i < num_values; i++) {
    Record *record = check_valid_parsed_tuple(command_table, values_parsed[i]);
    if (record == NULL) {
      printf("PROBLEM: Record could not be parsed\n");
      return false;
    }
    records[i] = *record;
  }

  // all good, time to add
  for (int i = 0; i < num_values; i++) {
    printf("record size to be added: %d\n", records[i].size);
    Page *p = add_record_to_page(schema, command_table, &records[i], buffer);
    if (p == NULL) {
      return false;
    }
  }

  // for (int i = 0; i < num_values; i++) {
  //   for (int j = 0; j < command_table->num_attributes; j++) {
  //     printf("%s ", values_parsed[i][j]);
  //   }
  //   printf("\n");
  // }
  return false;
}

void display_attributes(Schema *schema) {}

bool select_all(char *table_name, char *db_loc, Schema *schema,
                Bufferm *buffer) {
  Table *table = get_table(schema, table_name);
  if (table == NULL) {
    printf("No such table %s\n", table_name);
  }
  Page *p = find_in_buffer(buffer, table);
  char filepath[100];
  snprintf(filepath, sizeof(filepath), "%s/%s", schema->db_path, table->name);
  if (p == NULL) {
    p = read_page_from_file(schema, table, filepath);
    add_to_buffer(buffer, table, p, filepath);
  } else {
    write_page_to_file(table, p, filepath);
  }
  print_page(table, p);

  return true;
}

bool parse_select(char *command, char *db_loc, Schema *schema,
                  Bufferm *buffer) {
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
    printf("selecting all from %s ..\n", table_name);
    return select_all(table_name, db_loc, schema, buffer);
  }
  return true;
}

bool process_display_schema(char *command, char *db_loc, Schema *schema,
                            Bufferm *buffer) {
  for (int i = 0; i < schema->num_tables; i++) {
    Page *p = find_in_buffer(buffer, &schema->tables[i]);
    char filepath[100];
    snprintf(filepath, sizeof(filepath), "%s/%s", schema->db_path,
             schema->tables[i].name);
    if (p == NULL) {
      p = read_page_from_file(schema, &schema->tables[i], filepath);
      if (p != NULL) {
        add_to_buffer(buffer, &schema->tables[i], p, filepath);
      }
    }
    printf("Printing table %s\n", schema->tables[i].name);
    if (p != NULL) {
      print_page(&schema->tables[i], p);
    }
  }
  return false;
}

bool process_display_info(char *command, char *db_loc, Schema *schema) {
  char table_name[50];
  sscanf(command, "display info %s", &table_name);
  Table *table = get_table(schema, &table_name);
  printf("table %s not null\n", table_name);
  if (table == NULL) {
    printf("table cant be displayed, was null\n");
  } else {
    print_table_metadata(table);
  }

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

void parse_command(char *command, char *db_loc, Schema *schema,
                   Bufferm *buffer) {
  // Self explanatory code.
  if (startsWith(command, "select")) {
    parse_select(command, db_loc, schema, buffer);
  } else if (startsWith(command, "create")) {
    parse_create_table(command, db_loc, schema);
  } else if (startsWith(command, "insert")) {
    process_insert_record(command, db_loc, schema, buffer);
  } else if (startsWith(command, "display schema")) {
    process_display_schema(command, db_loc, schema, buffer);
  } else if (startsWith(command, "display info")) {
    process_display_info(command, db_loc, schema);
  } else {
    printf("Invalid command\n");
  }
  printf("\n");
}

void process(char *db_loc, Schema *schema, Bufferm *bufferm) {
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
        return;
      }
    }

    // Fill newlines with spaces
    while (strcspn(command, "\n") != strlen(command)) {
      command[strcspn(command, "\n")] = ' ';
    }

    // We've read a command from the user. Parse it and take some action
    parse_command(command, db_loc, schema, bufferm);

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
