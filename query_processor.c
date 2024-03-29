#ifndef QUERY_PROCESSOR_H
#define QUERY_PROCESSOR_H

#include "query_processor.h"
#include "attribute_types.h"
#include "catalog.h"
#include "conditional_parser.h"
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
      // printf("char len is %d\n", len);
      attribute_ptr->type = CHAR;
      attribute_ptr->len = len;
      // printf("char len is %d\n", attribute_ptr->len);
    }
    return CHAR;
  } else if (startsWith(
                 attr,
                 "varchar(")) { // need to check if of the form varchar(...
    int len;
    if (sscanf(attr, "varchar(%d)", &len) == 1) {
      attribute_ptr->type = VARCHAR;
      attribute_ptr->len = len;
      // printf("varchar len is %d\n", attribute_ptr->len);
    }
    return VARCHAR;
  }
  // parse failed
  printf("Invalid data type \"%s\"\n", attr);
  printf("ERROR\n");
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
    printf("Invalid table name\n");
    printf("ERROR\n");
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
    printf("ERROR\n");
    return false;
  }

  table_ptr->attributes = malloc(sizeof(Attribute) * 100);

  // printf("%s (len %lu) is table name \n", table_ptr->name,
  // strlen(table_ptr->name));

  // continue with rest of tokens
  table_ptr->num_attributes = 0;
  table_ptr->num_unique_attributes = 0;
  while (1) {
    table_ptr->num_attributes++;
    Attribute *attribute_ptr = malloc(sizeof(Attribute));
    attribute_ptr->is_primary_key = false;
    attribute_ptr->notnull = false;
    attribute_ptr->unique = false;

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

    // contains constraints
    ATTRIBUTE_TYPE attr_type =
        parse_attribute_type_before(token, 0, attribute_ptr);
    if (attr_type == INVALID_ATTR) {
      return false;
    }

    do {
      token = strtok(NULL, " ");
      if (startsWith(token, "primarykey")) {
        if (has_primary_key) {
          printf("Multiple primary keys found\n");
          printf("ERROR\n");
          return false;
        }
        has_primary_key = true;
        attribute_ptr->is_primary_key = true;
      } else if (startsWith(token, "notnull")) {
        attribute_ptr->notnull = true;
      } else if (startsWith(token, "unique")) {
        attribute_ptr->unique = true;
        table_ptr->num_unique_attributes++;
      } else {
        printf("Invalid constraint\n");
        printf("ERROR\n");
        return false;
      }
    } while (!(endsWith(token, ");") || endsWith(token, ",")));

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
    printf("ERROR ERROR ERROR\n");
    return false;
  }

  // check has primary key
  if (!has_primary_key) {
    printf("No primary key defined\n");
    printf("ERROR\n");
    return false;
  }

  // CHECK DUPLICATE ATTRIBUTE NAME
  for (int i = 0; i < table_ptr->num_attributes; i++) {
    for (int j = i + 1; j < table_ptr->num_attributes; j++) {
      if (strcmp(table_ptr->attributes[i].name,
                 table_ptr->attributes[j].name) == 0) {
        printf("Duplicate attribute name %s", table_ptr->attributes[i].name);
        printf("ERROR\n");
        return false;
      }
    }
  }

  // this updates the schema
  add_table_to_catalog(schema, table_ptr);

  // UNCOMMENT THIS ONLY IF U WANT TO TEST AND SEE OUTPUT
  // printf("the table that was just created: ..\n");
  // Schema *schema2 = read_catalog(db_loc);
  // for (int i = 0; i < schema2->num_tables; i++) {
  //  Table *curr_table = &schema2->tables[i];
  //  printf("table #%d name: %s\n", i, curr_table->name);
  //  for (int j = 0; j < curr_table->num_attributes; j++) {
  //    printf("attr #%d name: %s , type: %s , len: %d is_primary_key: %d\n", j,
  //           curr_table->attributes[j].name,
  //           attribute_type_to_string(curr_table->attributes[j].type),
  //           curr_table->attributes[j].len,
  //           curr_table->attributes[j].is_primary_key);
  //  }
  //}

  return true;
}

bool parse_tuple(char *tuple, char ***values_parsed, int tuple_index,
                 Table *command_table) {
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
          printf("Invalid data type, char is too long\n");
          printf("ERROR\n");
          return false;
        }
      } else {
        sscanf(next_tokens, "%s %[^\n]", current_token, next_tokens);
        null = true;
        if (strcmp(current_token, "null") != 0) {
          printf("Invalid data type, chars must be in quotes\n");
          printf("ERROR\n");
          return false;
        }
      }
    } else {
      sscanf(next_tokens, "%s %[^\n]", current_token, next_tokens);
      if (strcmp(current_token, "null") == 0) {
        null = true;
      } else {
        if (command_table->attributes[i].type == INTEGER) {
          char *pPosition = strchr(current_token, '.');
          if (pPosition != NULL) {
            printf("Cannot insert double for integer\n");
            printf("ERROR\n");
            return false;
          }
        }
      }
    }
    if (!null) {
      values_parsed[tuple_index][i] =
          (char *)malloc((strlen(current_token) + 1) * sizeof(char));
      strcpy(values_parsed[tuple_index][i], current_token);
    } else {
      if (command_table->attributes[i].notnull) {
        printf("Attribute cannot be null\n");
        printf("ERROR\n");
        return false;
      }
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
  int success =
      sscanf(command, "insert into %s values %[^;]", table_name, values);
  if (success != 2) {
    printf("Incorrect usage of insert. Please use: insert into TABLE_NAME "
           "values (VALUES) (VALUES)...\n");
    printf("ERROR\n");
    return false;
  }

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
  if (command_table == NULL) {
    printf("No such table %s\n", table_name);
    printf("ERROR\n");
    return false;
  }
  char ***values_parsed;
  values_parsed = (char ***)malloc(num_values * sizeof(char **));
  for (int i = 0; i < num_values; i++) {
    values_parsed[i] =
        (char **)malloc(command_table->num_attributes * sizeof(char *));
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
  // printf("num values: %d\n", num_values);
  // printf("printing values parsed\n");

  Record *records = malloc(sizeof(Record) * num_values);

  // validate
  for (int i = 0; i < num_values; i++) {
    Record *record = check_valid_parsed_tuple(command_table, values_parsed[i]);
    if (record == NULL) {
      printf("ERROR\n");
      return false;
    }
    records[i] = *record;
  }

  // all good, time to add
  for (int i = 0; i < num_values; i++) {
    // printf("record size to be added: %d\n", records[i].size);
    // printf("adding %i \n", i);
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
    printf("ERROR\n");
    return false;
  }
  Page *p = find_in_buffer(buffer, table);
  // printf("num records of first page: %d\n", p->num_records);
  char filepath[100];
  snprintf(filepath, sizeof(filepath), "%s/%s", schema->db_path, table->name);
  if (p == NULL) {
    // printf("select all reading from page file\n");
    p = read_page_from_file(schema, table, filepath);
    if (p != NULL) {
      add_to_buffer(buffer, table, p, filepath);
    }
  }
  printf("| ");
  for (int i = 0; i < table->num_attributes; i++) {
    printf("%s | ", table->attributes[i].name);
  }
  printf("\n");
  if (p != NULL) {
    print_page(table, p);
  }

  return true;
}

bool select_all_where(char *table_name, char *db_loc, Schema *schema,
                      Bufferm *buffer,
                      ConditionalParseTree *conditionalParseTree) {
  Table *table = get_table(schema, table_name);
  if (table == NULL) {
    printf("No such table %s\n", table_name);
    printf("ERROR\n");
    return false;
  }
  Page *p = find_in_buffer(buffer, table);
  // printf("num records of first page: %d\n", p->num_records);
  char filepath[100];
  snprintf(filepath, sizeof(filepath), "%s/%s", schema->db_path, table->name);
  if (p == NULL) {
    // printf("select all reading from page file\n");
    p = read_page_from_file(schema, table, filepath);
    if (p != NULL) {
      add_to_buffer(buffer, table, p, filepath);
    }
  }
  printf("| ");
  for (int i = 0; i < table->num_attributes; i++) {
    printf("%s | ", table->attributes[i].name);
  }
  printf("\n");
  if (p != NULL) {
    // print_page(table, p);
    print_page_where(table, p, conditionalParseTree);
  }

  return true;
}

bool select_where_projection(char *table_name, char *db_loc, Schema *schema,
                             Bufferm *buffer,
                             ConditionalParseTree *conditionalParseTree,
                             char **requested_attributes,
                             int num_attributes_requested) {
  Table *table = get_table(schema, table_name);
  if (table == NULL) {
    printf("No such table %s\n", table_name);
    printf("ERROR\n");
    return false;
  }
  Page *p = find_in_buffer(buffer, table);
  // printf("num records of first page: %d\n", p->num_records);
  char filepath[100];
  snprintf(filepath, sizeof(filepath), "%s/%s", schema->db_path, table->name);
  if (p == NULL) {
    // printf("select all reading from page file\n");
    p = read_page_from_file(schema, table, filepath);
    if (p != NULL) {
      add_to_buffer(buffer, table, p, filepath);
    }
  }
  printf("| ");
  for (int i = 0; i < num_attributes_requested; i++) {
    printf("%s | ", requested_attributes[i]);
  }
  printf("\n");
  if (p != NULL) {
    // print_page(table, p);
    print_page_where_projection(table, p, conditionalParseTree,
                                requested_attributes, num_attributes_requested);
  }

  return true;
}

bool select_where_projection_product(char **table_names,
                                     int num_tables_requested, char *db_loc,
                                     Schema *schema, Bufferm *buffer,
                                     ConditionalParseTree *conditionalParseTree,
                                     char **requested_attributes,
                                     int num_attributes_requested) {

  Table *table = get_table(schema, table_names[0]);
  if (table == NULL) {
    printf("No such table %s\n", table_names[0]);
    printf("ERROR\n");
    return false;
  }
  for (int i = 1; i < num_tables_requested; i++) {
    char filepath[100];
    snprintf(filepath, sizeof(filepath), "%s/%s", schema->db_path, table->name);
    Page *p1 = read_page_from_file(schema, table, filepath);
    Table *right_table = get_table(schema, table_names[i]);
    if (table == NULL) {
      printf("No such table %s\n", table_names[i]);
      printf("ERROR\n");
      return false;
    }
    char filepath2[100];
    snprintf(filepath2, sizeof(filepath2), "%s/%s", schema->db_path,
             right_table->name);
    Page *p2 = read_page_from_file(schema, right_table, filepath2);
    table = join_two_tables_block_nested(table, right_table, p1, p2, schema,
                                         buffer);
    add_table_to_catalog(schema, table);
  }

  Page *p = find_in_buffer(buffer, table);
  // printf("num records of first page: %d\n", p->num_records);
  char filepath[100];
  snprintf(filepath, sizeof(filepath), "%s/%s", schema->db_path, table->name);
  if (p == NULL) {
    // printf("select all reading from page file\n");
    p = read_page_from_file(schema, table, filepath);
    if (p != NULL) {
      add_to_buffer(buffer, table, p, filepath);
    }
  }
  printf("| ");
  for (int i = 0; i < num_attributes_requested; i++) {
    printf("%s | ", requested_attributes[i]);
  }
  printf("\n");
  if (p != NULL) {
    // print_page(table, p);
    print_page_where_projection(table, p, conditionalParseTree,
                                requested_attributes, num_attributes_requested);
  }

  return true;
}

bool select_all_where_product(char **table_names, int num_tables_requested,
                              char *db_loc, Schema *schema, Bufferm *buffer,
                              ConditionalParseTree *conditionalParseTree) {

  Table *table = get_table(schema, table_names[0]);
  if (table == NULL) {
    printf("No such table %s\n", table_names[0]);
    printf("ERROR\n");
    return false;
  }
  for (int i = 1; i < num_tables_requested; i++) {
    // Page * p1 = read_page_from_file(schema, table, filepath);
    Page *p1 = find_in_buffer(buffer, table);
    // printf("num records of first page: %d\n", p->num_records);
    char filepath[100];
    snprintf(filepath, sizeof(filepath), "%s/%s", schema->db_path, table->name);
    if (p1 == NULL) {
      // printf("select all reading from page file\n");
      p1 = read_page_from_file(schema, table, filepath);
      if (p1 != NULL) {
        add_to_buffer(buffer, table, p1, filepath);
      }
    }

    Table *right_table = get_table(schema, table_names[i]);
    if (table == NULL) {
      printf("No such table %s\n", table_names[i]);
      printf("ERROR\n");
      return false;
    }

    // Page * p2 = read_page_from_file(schema, right_table, filepath2);
    Page *p2 = find_in_buffer(buffer, right_table);
    // printf("num records of first page: %d\n", p->num_records);
    char filepath2[100];
    snprintf(filepath2, sizeof(filepath2), "%s/%s", schema->db_path,
             right_table->name);

    if (p2 == NULL) {
      // printf("select all reading from page file\n");
      p2 = read_page_from_file(schema, right_table, filepath);
      if (p2 != NULL) {
        add_to_buffer(buffer, right_table, p2, filepath);
      }
    }

    table = join_two_tables_block_nested(table, right_table, p1, p2, schema,
                                         buffer);
    add_table_to_catalog(schema, table);
  }

  Page *p = find_in_buffer(buffer, table);

  // printf("num records of first page: %d\n", p->num_records);
  char filepath[100];
  snprintf(filepath, sizeof(filepath), "%s/%s", schema->db_path, table->name);
  if (p == NULL) {
    // printf("select all reading from page file\n");
    p = read_page_from_file(schema, table, filepath);
    if (p != NULL) {
      add_to_buffer(buffer, table, p, filepath);
    }
  }
  printf("| ");
  for (int i = 0; i < table->num_attributes; i++) {
    printf("%s | ", table->attributes[i].name);
  }
  printf("\n");
  if (p != NULL) {
    // print_page(table, p);
    print_page_where(table, p, conditionalParseTree);
  }

  return true;
}

bool select_where_projection_product_orderby(
    char **table_names, int num_tables_requested, char *db_loc, Schema *schema,
    Bufferm *buffer, ConditionalParseTree *conditionalParseTree,
    char **requested_attributes, int num_attributes_requested,
    char *orderby_attr) {
  Table *table = get_table(schema, table_names[0]);
  if (table == NULL) {
    printf("No such table %s\n", table_names[0]);
    printf("ERROR\n");
    return false;
  }
  for (int i = 1; i < num_tables_requested; i++) {
    // Page * p1 = read_page_from_file(schema, table, filepath);
    Page *p1 = find_in_buffer(buffer, table);
    // printf("num records of first page: %d\n", p->num_records);
    char filepath[100];
    snprintf(filepath, sizeof(filepath), "%s/%s", schema->db_path, table->name);
    if (p1 == NULL) {
      // printf("select all reading from page file\n");
      p1 = read_page_from_file(schema, table, filepath);
      if (p1 != NULL) {
        add_to_buffer(buffer, table, p1, filepath);
      }
    }

    Table *right_table = get_table(schema, table_names[i]);
    if (table == NULL) {
      printf("No such table %s\n", table_names[i]);
      printf("ERROR\n");
      return false;
    }

    // Page * p2 = read_page_from_file(schema, right_table, filepath2);
    Page *p2 = find_in_buffer(buffer, right_table);
    // printf("num records of first page: %d\n", p->num_records);
    char filepath2[100];
    snprintf(filepath2, sizeof(filepath2), "%s/%s", schema->db_path,
             right_table->name);

    if (p2 == NULL) {
      // printf("select all reading from page file\n");
      p2 = read_page_from_file(schema, right_table, filepath);
      if (p2 != NULL) {
        add_to_buffer(buffer, right_table, p2, filepath);
      }
    }

    table = join_two_tables_block_nested(table, right_table, p1, p2, schema,
                                         buffer);
    add_table_to_catalog(schema, table);
  }

  Page *p = find_in_buffer(buffer, table);

  // printf("num records of first page: %d\n", p->num_records);
  char filepath[100];
  snprintf(filepath, sizeof(filepath), "%s/%s", schema->db_path, table->name);
  if (p == NULL) {
    // printf("select all reading from page file\n");
    p = read_page_from_file(schema, table, filepath);
    if (p != NULL) {
      add_to_buffer(buffer, table, p, filepath);
    }
  }
  printf("| ");
  for (int i = 0; i < num_attributes_requested; i++) {
    printf("%s | ", requested_attributes[i]);
  }
  printf("\n");
  if (p != NULL) {
    // print_page(table, p);
    print_page_where_projection_orderby(table, p, conditionalParseTree,
                                        requested_attributes,
                                        num_attributes_requested, orderby_attr);
  }

  return true;
}

bool select_all_where_product_orderby(
    char **table_names, int num_tables_requested, char *db_loc, Schema *schema,
    Bufferm *buffer, ConditionalParseTree *conditionalParseTree,
    char *orderby_attr) {
  Table *table = get_table(schema, table_names[0]);
  if (table == NULL) {
    printf("No such table %s\n", table_names[0]);
    printf("ERROR\n");
    return false;
  }
  for (int i = 1; i < num_tables_requested; i++) {
    // Page * p1 = read_page_from_file(schema, table, filepath);
    Page *p1 = find_in_buffer(buffer, table);
    // printf("num records of first page: %d\n", p->num_records);
    char filepath[100];
    snprintf(filepath, sizeof(filepath), "%s/%s", schema->db_path, table->name);
    if (p1 == NULL) {
      // printf("select all reading from page file\n");
      p1 = read_page_from_file(schema, table, filepath);
      if (p1 != NULL) {
        add_to_buffer(buffer, table, p1, filepath);
      }
    }

    Table *right_table = get_table(schema, table_names[i]);
    if (table == NULL) {
      printf("No such table %s\n", table_names[i]);
      printf("ERROR\n");
      return false;
    }

    // Page * p2 = read_page_from_file(schema, right_table, filepath2);
    Page *p2 = find_in_buffer(buffer, right_table);
    // printf("num records of first page: %d\n", p->num_records);
    char filepath2[100];
    snprintf(filepath2, sizeof(filepath2), "%s/%s", schema->db_path,
             right_table->name);

    if (p2 == NULL) {
      // printf("select all reading from page file\n");
      p2 = read_page_from_file(schema, right_table, filepath);
      if (p2 != NULL) {
        add_to_buffer(buffer, right_table, p2, filepath);
      }
    }

    table = join_two_tables_block_nested(table, right_table, p1, p2, schema,
                                         buffer);
    add_table_to_catalog(schema, table);
  }

  Page *p = find_in_buffer(buffer, table);

  // printf("num records of first page: %d\n", p->num_records);
  char filepath[100];
  snprintf(filepath, sizeof(filepath), "%s/%s", schema->db_path, table->name);
  if (p == NULL) {
    // printf("select all reading from page file\n");
    p = read_page_from_file(schema, table, filepath);
    if (p != NULL) {
      add_to_buffer(buffer, table, p, filepath);
    }
  }
  printf("| ");
  for (int i = 0; i < table->num_attributes; i++) {
    printf("%s | ", table->attributes[i].name);
  }
  printf("\n");
  if (p != NULL) {
    // print_page(table, p);
    print_page_where_product_orderby(table, p, conditionalParseTree,
                                     orderby_attr);
  }

  return true;
}

bool parse_select(char *command, char *db_loc, Schema *schema,
                  Bufferm *buffer) {
  /*
   * SELECT <attrs> FROM <table_name> WHERE <conditionals>
   */
  char table_name[256];
  char *token = strtok(command, " "); // select
  char **attributes = malloc(sizeof(char *) * 256);
  int num_attributes = 0;
  token = strtok(NULL, " "); // <attr1>
  while (strcmp(token, "from") != 0) {
    attributes[num_attributes] = malloc(256);
    attributes[num_attributes][0] = '\0';
    strcpy(attributes[num_attributes], token);
    token = strtok(NULL, " ");
    num_attributes++;
  }
  // Remove the commas from the attribute names
  for (int i = 0; i < num_attributes; i++) {
    if (attributes[i][strlen(attributes[i]) - 1] == ',') {
      attributes[i][strlen(attributes[i]) - 1] = '\0';
    }
  }

  if (strcmp(token, "from") != 0) {
    printf("Syntax Error\n");
    return false;
  }

  char **table_names = malloc(sizeof(char *) * 256);
  int num_tables_requested = 0;
  token = strtok(NULL, " "); // <table_name1>

  while (token && token[strlen(token) - 1] != ';' &&
         strcmp(token, "where") != 0 && strcmp(token, "groupby") != 0 &&
         strcmp(token, "orderby") != 0) {
    table_names[num_tables_requested] = malloc(256);
    table_names[num_tables_requested][0] = '\0';
    strcpy(table_names[num_tables_requested], token);
    token = strtok(NULL, " ");
    num_tables_requested++;
  }
  if (table_names[num_tables_requested - 1]
                 [strlen(table_names[num_tables_requested - 1]) - 1] == ';') {
    table_names[num_tables_requested - 1]
               [strlen(table_names[num_attributes - 1]) - 1] = '\0';
  }

  // Remove the commas from the table names
  for (int i = 0; i < num_tables_requested; i++) {
    if (table_names[i][strlen(table_names[i]) - 1] == ',') {
      table_names[i][strlen(table_names[i]) - 1] = '\0';
    }
  }

  // strcpy(table_name, token);
  //  printf("tableName: %s\n", table_name);

  // token = strtok(NULL, " "); // where
  char *condition = malloc(250);
  if (token && strcmp(token, "where") == 0) {
    token = strtok(NULL, " "); // <condition>
    // parse condition
    while (token != NULL && token[strlen(token) - 1] != ';' &&
           strcmp(token, "groupby") != 0 && strcmp(token, "orderby") != 0) {
      strcat(condition, token);
      condition[strlen(condition) + 1] = '\0';
      condition[strlen(condition)] = ' ';
      token = strtok(NULL, " ");
    }
  } else {
    condition[0] = '\0';
    strcat(condition, "true");
    condition[4] = '\0';
  }

  // parse semicolon
  if (condition[strlen(condition) - 1] == ';') {
    condition[strlen(condition) - 1] = '\0';
  }

  ConditionalParseTree *conditionTree = parseConditional(condition);
  bool groupby_query = false;
  char *groupby_attr = NULL;
  bool orderby_query = false;
  char *orderby_attr = NULL;
  if (token && token[strlen(token) - 1] != ';') {
    if (startsWith(token, "groupby") == true) {
      groupby_query = true;
      token = strtok(NULL, " "); // <groupby_attr>
      groupby_attr = malloc(250);
      strcpy(groupby_attr, token);
      //printf("Group By: %s\n", groupby_attr);
      token = strtok(NULL, " "); // orderby
    }
  }

  if (token && token[strlen(token) - 1] != ';') {
    if (startsWith(token, "orderby") == true) {
      orderby_query = true;
      token = strtok(NULL, " "); // <orderby_attr>
      orderby_attr = malloc(250);
      strcpy(orderby_attr, token);
      token = strtok(NULL, " ");
    } else {
      printf("Syntax Error\n");
      return false;
    }
  }

  if (strcmp(attributes[0], "*") == 0) {
    // printf("selecting all from %s ..\n", table_name);
    // return select_all(table_name, db_loc, schema, buffer);
    if (orderby_query == true) {
      return select_all_where_product_orderby(table_names, num_tables_requested,
                                              db_loc, schema, buffer,
                                              conditionTree, orderby_attr);
    }
    return select_all_where_product(table_names, num_tables_requested, db_loc,
                                    schema, buffer, conditionTree);
  }

  if (orderby_query == true) {
    return select_where_projection_product_orderby(
        table_names, num_tables_requested, db_loc, schema, buffer,
        conditionTree, attributes, num_attributes, orderby_attr);
  }

  return select_where_projection_product(table_names, num_tables_requested,
                                         db_loc, schema, buffer, conditionTree,
                                         attributes, num_attributes);
}

bool process_display_schema(char *command, char *db_loc, Schema *schema,
                            Bufferm *buffer) {
  printf("DB location: %s\n", db_loc);
  printf("Page Size : %d\n", schema->page_size);
  printf("Buffer Size: %d\n", schema->buffer_size);
  printf("\n");
  printf("Tables:\n");
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
    print_table_metadata(&schema->tables[i]);
    int page_count = 0;
    int record_count = 0;
    while (p != NULL) {
      page_count += 1;
      record_count += p->num_records;
      if (p->next_page != NULL) {
        p = p->next_page;
      } else {
        break;
      }
    }
    printf("Pages: %d\n", page_count);
    printf("Records: %d\n", record_count);
    // if (p != NULL) {
    //   print_page(&schema->tables[i], p);
    // }
  }
  return false;
}

bool process_display_info(char *command, char *db_loc, Schema *schema,
                          Bufferm *buffer) {
  char table_name[50];
  sscanf(command, "display info %s", &table_name);
  Table *table = get_table(schema, &table_name);
  if (table == NULL) {
    printf("No such table %s\n", table_name);
    printf("ERROR\n");
  } else {
    print_table_metadata(table);
    Page *p = find_in_buffer(buffer, table);
    char filepath[100];
    snprintf(filepath, sizeof(filepath), "%s/%s", schema->db_path, table->name);
    if (p == NULL) {
      p = read_page_from_file(schema, table, filepath);
      if (p != NULL) {
        add_to_buffer(buffer, table, p, filepath);
      }
    }
    int page_count = 0;
    int record_count = 0;
    while (p != NULL) {
      page_count += 1;
      record_count += p->num_records;
      if (p->next_page != NULL) {
        p = p->next_page;
      } else {
        break;
      }
    }
    printf("Pages: %d\n", page_count);
    printf("Records: %d\n", record_count);
  }

  return false;
}

bool process_drop_table(char *command, char *db_loc, Schema *schema,
                        Bufferm *buffer) {
  /*
   * drop table <table_name>
   */
  char *token = strtok(command, " ");
  token = strtok(NULL, " ");
  if (strcmp(token, "table") != 0) {
    printf("Syntax Error\n");
    return false;
  }
  token = strtok(NULL, " ");
  char *table_name = malloc(strlen(token) + 1);
  strcpy(table_name, token);
  //printf("Dropping table %s\n", table_name);
  return drop_table(schema, buffer, table_name);
}

void save_catalog(Schema *schema, char *db_loc) {
  char path[100];
  strcpy(path, db_loc);
  strcat(path, "/catalog");
  FILE *fp = fopen(path, "wb");
  fwrite(&(*schema), sizeof(Schema), 1, fp);
}

bool parse_alter_table(char *command, char *db_loc, Schema *schema,
                       Bufferm *buffer) {
  // Alter table <table_name> add <attr_name> <type>;
  // Alter table <table_name> add <attr_name> <type> default <value>;
  // Alter table <table_name> drop <attr_name>;

  command[strlen(command) + 1] = '\0';
  command[strlen(command)] = ';';

  char *token = strtok(command, " "); // "alter"

  token = strtok(NULL, " "); // "table"

  if (startsWith(token, "table") == false) {
    printf("Error\n");
    return false;
  }

  token = strtok(NULL, " "); // <table_name>

  char *table_name = malloc(strlen(token) + 1);
  strcpy(table_name, token);

  token = strtok(NULL, " "); // "add" or "drop"

  if (strcmp(token, "add") == 0) {
    // printf("Adding new attribute\n");
    token = strtok(NULL, " "); // <attr_name>
    char *attr_name = malloc(strlen(token) + 1);
    strcpy(attr_name, token);
    token = strtok(NULL, " "); // <type>
    char *attr_type = malloc(strlen(token) + 1);
    strcpy(attr_type, token);
    Attribute *attr = malloc(sizeof(Attribute));
    char *default_value = "null"; // default value is null
    // printf("setting name\n");
    attr->name = malloc(strlen(attr_name) + 1);
    strcpy(attr->name, attr_name);
    if (attr_type[strlen(attr_type) - 1] == ';') {
      // printf("default is null with ; at end of attr_name\n");
      attr_type[strlen(attr_type) - 1] = '\0';
      // printf("setting name\n");
      attr->name = malloc(strlen(attr_name) + 1);
      strcpy(attr->name, attr_name);
      // printf("here\n");
      ATTRIBUTE_TYPE t =
          parse_attribute_type(attr_type, attr); // parse the type
      if (t == INVALID_ATTR) {
        return false;
      }
      attr->is_primary_key = false;
    } else {
      ATTRIBUTE_TYPE t =
          parse_attribute_type(attr_type, attr); // parse the type
      token = strtok(NULL, " ");                 // "default"
      if (strcmp(token, ";") == 0) {
        if (t == INVALID_ATTR) {
          return false;
        }
        attr->is_primary_key = false;
      } else if (startsWith(token, "default") == true) {
        token = strtok(NULL, " "); // <value>
        default_value = malloc(strlen(token) + 1);
        strcpy(default_value, token);
      }
    }
    Attribute_Values *attr_values_ptr = malloc(sizeof(Attribute_Values));
    attr_values_ptr->type = attr->type;

    // printf("%s is default_value to be parsed..\n", default_value);

    if (startsWith(default_value, "null")) {
      attr_values_ptr = NULL;
      // printf("default value will be null\n");
    } else if (startsWith(attr_type, "integer")) {
      int default_int_value;
      sscanf(default_value, "%d", &default_int_value);
      attr_values_ptr = malloc(sizeof(Attribute_Values));
      attr_values_ptr->int_val = default_int_value;
      // printf("default int value: %d\n", attr_values_ptr->int_val);
    } else if (startsWith(attr_type, "double")) {
      double default_double_value;
      sscanf(default_value, "%lf", &default_double_value);
      attr_values_ptr->double_val = default_double_value;
      // printf("default double value: %f\n", attr_values_ptr->double_val);
    } else if (startsWith(attr_type, "char")) {
      char *default_char_value = malloc(strlen(default_value) + 1);
      strcpy(default_char_value, default_value);
      if (default_char_value[0] != '\"') {
        printf("Default value missing left quotes\n");
        printf("Error\n");
        return false;
      }

      default_char_value += 1; // remove the first quote

      int orig_len = strlen(default_char_value);

      if (default_char_value[orig_len - 1] == ';') {
        default_char_value[orig_len - 1] = '\0';
      }
      if (default_char_value[orig_len - 2] != '\"') {
        printf("Default value missing right quotes\n");
        printf("Error\n");
        return false;
      }

      default_char_value[strlen(default_char_value) - 1] =
          ' '; // remove the last quote
      // int targetStrLen = attr->len;
      // const char *padding = "x";
      // int padLen = targetStrLen - strlen(default_char_value);
      // if (padLen < 0) {
      //   padLen = 0;
      // }

      char *temp = malloc(sizeof(char) * attr->len + 1);
      strncpy(temp, default_char_value, strlen(default_char_value));
      temp[attr->len] = '\0';
      attr_values_ptr->chars_val = temp;
      // printf("default char value: %s\n", temp);
      // printf("chars len is : %lu but it should be %d\n", strlen(temp),
      //        attr->len);

    } else if (startsWith(attr_type, "varchar")) {
      char *default_varchar_value = malloc(strlen(default_value) + 1);
      strcpy(default_varchar_value, default_value);
      if (default_varchar_value[0] != '\"') {
        printf("Default value missing left quotes\n");
        printf("Error\n");
        return false;
      }
      default_varchar_value += 1; // remove the first quote

      if (default_varchar_value[strlen(default_varchar_value) - 1] == ';') {
        default_varchar_value[strlen(default_varchar_value) - 1] = '\0';
      }
      if (default_varchar_value[strlen(default_varchar_value) - 1] != '\"') {
        printf("Default value missing right quotes\n");
        printf("Error\n");
        return false;
      }
      default_varchar_value[strlen(default_varchar_value) - 1] =
          '\0'; // remove the last quote
      attr_values_ptr->chars_val = default_varchar_value;
      // printf("default varchar value: %s\n", attr_values_ptr->chars_val);
    }
    // printf("attribute name: %s\n", attr->name);
    return alter_table_add(schema, buffer, table_name, attr, attr_values_ptr);

  }

  else if (strcmp(token, "drop") == 0) {
    token = strtok(NULL, " "); // <attr_name>
    char *attr_name = malloc(strlen(token) + 1);
    strcpy(attr_name, token);
    if (attr_name[strlen(attr_name) - 1] == ';') {
      attr_name[strlen(attr_name) - 1] = '\0';
    }
    // printf("attr name: %s\n", attr_name);
    return alter_table_drop(schema, buffer, table_name, attr_name);
  }

  else {
    printf("Error\n");
    return false;
  }
}

bool parse_delete(char *command, char *db_loc, Schema *schema,
                  Bufferm *buffer) {
  /*
   * delete from <table_name> where <condition>;
   */
  char *token = strtok(command, " "); // delete
  token = strtok(NULL, " ");          // from

  if (strcmp(token, "from") != 0) {
    printf("Syntax Error");
    return false;
  }
  token = strtok(NULL, " "); // <table_name>
  char *table_name = malloc(strlen(token) + 1);
  strcpy(table_name, token);
  Table *original_table = get_table(schema, table_name);
  if (original_table == NULL) {
    printf("table %s does not exist\n", table_name);
    return false;
  }
  token = strtok(NULL, " "); // where
  char *condition = malloc(250);
  // If no where clause, condition is true
  if (!token || endsWith(token, ";") != 0) {
    // parse semicolon
    if (table_name[strlen(table_name) - 1] == ';') {
      table_name[strlen(table_name) - 1] = '\0';
    }

    condition[0] = '\0';
    strcat(condition, "true");
    // For Testing
    printf("table name: %s\n", table_name);
    printf("condition: %s\n", condition);
    if (drop_table(schema, buffer, table_name)) {
      add_table_to_catalog(schema, original_table);
    } else {
      // idk
    }
    return true;
  }

  else if (strcmp(token, "where") != 0) {
    printf("Syntax Error");
    return false;
  }

  token = strtok(NULL, " "); // <condition>

  // condition is true if there is no condition
  if (endsWith(token, ";") == true) {
    condition[0] = '\0';
    strcat(condition, "true");
  }

  // parse condition
  while (token != NULL && token[strlen(token) - 1] != ';') {
    strcat(condition, token);
    condition[strlen(condition) + 1] = '\0';
    condition[strlen(condition)] = ' ';
    token = strtok(NULL, " ");
  }

  // parse semicolon
  if (condition[strlen(condition) - 1] == ';') {
    condition[strlen(condition) - 1] = '\0';
  }

  // For Testing
  //printf("table name: %s\n", table_name);
  //printf("condition: %s\n", condition);
  // char * conditionCopy = malloc(strlen(condition)+1);
  if (condition[strlen(condition) - 1] == '\n') {
    condition[strlen(condition) - 1] = '\0';
  }

  // strcpy(conditionCopy, condition);
  // condition[strlen(condition)] = '\0';

  ConditionalParseTree *conditionTree = parseConditional(condition);
  //printf("Conditional Parse Tree:\n");
  //printConditionalParseTree(conditionTree);

  char filepath[100];
  snprintf(filepath, sizeof(filepath), "%s/%s", schema->db_path, table_name);
  Page *p = find_in_buffer(buffer, original_table);
  if (p == NULL) {
    p = read_page_from_file(schema, original_table, filepath);
    if (p == NULL) {
      printf("didnt find page for %s while deleting\n", table_name);
    }
  }

  //printf("origional table: %s\n", original_table->name);

  // drop the old table
  // we still have reference to it tho via original_table
  if (drop_table(schema, buffer, table_name)) {
    add_table_to_catalog(schema, original_table);
  } else {
    // idk
  }
  return delete_where(schema, original_table, buffer, p, conditionTree);
}

bool parse_update_table(char *command, char *db_loc, Schema *schema,
                        Bufferm *buffer) {
  /*
   * update <table_name> set <column_1> = <value> where <condition>;
   */

  char *token = strtok(command, " "); // update
  token = strtok(NULL, " ");          // <table_name>
  char *table_name = malloc(strlen(token) + 1);
  strcpy(table_name, token);
  Table *original_table = get_table(schema, table_name);
  if (original_table == NULL) {
    printf("table %s does not exist\n", table_name);
    return false;
  }
  token = strtok(NULL, " "); // set
  if (strcmp(token, "set") != 0) {
    printf("Syntax Error\n");
    return false;
  }

  token = strtok(NULL, " "); // <column_1>
  char *column = malloc(strlen(token) + 1);
  strcpy(column, token);

  token = strtok(NULL, " "); // =
  if (strcmp(token, "=") != 0) {
    printf("Syntax Error\n");
    return false;
  }

  token = strtok(NULL, " "); // <value>
  char *value = malloc(strlen(token) + 1);
  strcpy(value, token);
  // parse quotes if there are any
  if(value[0] == '\"'){
      value += 1;
      value[strlen(value) - 1] = '\0';
  }

  token = strtok(NULL, " "); // where

  char *condition = malloc(256);
  // condition is true if no where clause
  if (!token || endsWith(token, ";")) {
    // parse semicolon from value
    if (value[strlen(value) - 1] == ';') {
      value[strlen(value) - 1] = '\0';
    }
    condition[0] = '\0';
    strcat(condition, "true");
    printf("table name: %s\n", table_name);
    printf("column: %s\n", column);
    printf("value: %s\n", value);
    printf("condition: %s\n", condition);
    return true;
  } else if (strcmp(token, "where") != 0) {
    printf("Syntax Error\n");
    return false;
  }
  token = strtok(NULL, " "); // <condition>

  condition[0] = '\0';
  while (token && endsWith(token, ";") == false) {
    strcat(condition, token);
    condition[strlen(condition) + 1] = '\0';
    condition[strlen(condition)] = ' ';
    token = strtok(NULL, " ");
  }

  if (condition[strlen(condition) - 1] == ';') {
    condition[strlen(condition) - 1] = '\0';
  }
  //printf("table name: %s\n", table_name);
  //printf("column: %s\n", column);
  //printf("value: %s\n", value);
  //printf("condition: %s\n", condition);

  ConditionalParseTree *conditionTree = parseConditional(condition);
  //printf("Conditional Parse Tree:\n");
  printConditionalParseTree(conditionTree);
    
  char filepath[100];
  snprintf(filepath, sizeof(filepath), "%s/%s", schema->db_path, table_name);
  Page *p = find_in_buffer(buffer, original_table);
  if (p == NULL) {
    p = read_page_from_file(schema, original_table, filepath);
    if (p == NULL) {
      printf("didnt find page for %s while deleting\n", table_name);
    }
  }

  int attribute_index = -1;
  for (int i = 0; i < original_table->num_attributes; i++) {
    if (strcmp(column, original_table->attributes[i].name) == 0){
      attribute_index = i;
    }
  }
  if (attribute_index == -1){
    printf("Column %s not found.\n Error.\n", table_name);
    return false;
  }
  //printf("Attribute index is %d\n", attribute_index);
    
  // drop the old table
  // we still have reference to it tho via original_table
  if (drop_table(schema, buffer, table_name)) {
    add_table_to_catalog(schema, original_table);
  } else {
    // idk
  }
  return update_where(schema, original_table, buffer, p, conditionTree, attribute_index, value);    
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
    process_display_info(command, db_loc, schema, buffer);
  } else if (startsWith(command, "drop")) {
    process_drop_table(command, db_loc, schema, buffer);
  } else if (startsWith(command, "alter")) {
    parse_alter_table(command, db_loc, schema, buffer);
  } else if (startsWith(command, "delete")) {
    parse_delete(command, db_loc, schema, buffer);
  } else if (startsWith(command, "update")) {
    parse_update_table(command, db_loc, schema, buffer);
  } else {
    printf("Invalid command\n");
  }
  printf("\n");
}

void process(char *db_loc, Schema *schema, Bufferm *buffer) {
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
        printf("Purging page buffer...\n");
        printf("Saving catalog...\n");
        printf("\nExiting the database...\n");
        // save table schemas to catalog
        write_schemas_to_catalog(schema);
        // flush page buffer and write to disk
        flush_buffer(buffer);
        return;
      }
    }

    // Fill newlines with spaces
    while (strcspn(command, "\n") != strlen(command)) {
      command[strcspn(command, "\n")] = ' ';
    }

    // We've read a command from the user. Parse it and take some action
    parse_command(command, db_loc, schema, buffer);

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
