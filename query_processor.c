#include "query_processor.h"
#include "attribute_types.h"
#include "parse_utils.h"
#include "table.h"

ATTRIBUTE_TYPE parse_attribute_type_before(char *attr,
                                           int num_last_char_exclude,
                                           Attribute *attribute_ptr) {
  char temp[strlen(attr) - num_last_char_exclude + 1];
  strncpy(temp, attr, strlen(attr) - num_last_char_exclude);
  return parse_attribute_type(temp, attribute_ptr);
}

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

/*
 * WORK IN PROGRESS
 *
 * */
bool process_create_table() {
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

bool process_select() {
  char select_query[1000];
  char table[1000];
  fgets(select_query, 1000, stdin);

  // remove trailing newline
  select_query[strlen(select_query) - 1] = '\0';

  // need that space in the beginning since "select" was parsed in process()
  // and the rest is of the form " ..."
  if (sscanf(select_query, " * from %s;", table) == 1) {
    printf("%s is table \n", table);
    return true;
  }
  return false;
}

void process() {
  char word[100];
  while (1) {
    scanf("%s", word);
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

void query_loop() { process(); }
