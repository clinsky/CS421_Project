#ifndef QUERY_PROCESSOR_H
#define QUERY_PROCESSOR_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
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




bool process_create_table(char * table_name, int num_attributes, char ** attribute_strings, char * db_loc, Schema * schema) {

    int MAX_NAME_LEN = 100;
    bool has_primary_key = false;

    Table *table_ptr = (Table *)malloc(sizeof(Table));

    /*
    if (!endsWith(table_name, "(")) {
        // should follow format create table foo(
        printf("invalid table name\n");
        return false;
    }
     */



    table_ptr->name = table_name;

    for (int i = 0; i < num_attributes; i++) {

        char attribute_name[256];
        char attribute_type[256];
        char * attribute_string = attribute_strings[i];
        char * tokens = strtok(attribute_string, " ");

        strcpy(attribute_name, tokens);
        tokens = strtok(NULL, " ");
        strcpy(attribute_type, tokens);

        tokens = strtok(NULL, " ");

        if (tokens != NULL && strcmp(tokens, "primarykey") == 0) {
            // TODO: Make this attribute the primary key
            if (has_primary_key) {
                printf("More than one primarykey\n");
                return false;
            }
            has_primary_key = true;
        }


        Attribute attributes[num_attributes];
        table_ptr->attributes = attributes;
        Attribute * attribute_ptr = (Attribute *)malloc(sizeof(Attribute));
        table_ptr->attributes[i] = *attribute_ptr;
        attribute_ptr->name = attribute_name;
        attribute_ptr->type = parse_attribute_type(attribute_type, attribute_ptr);
        (table_ptr->attributes)[i] = *attribute_ptr;
        table_ptr->num_attributes++;
        unsigned int num_tables = schema->num_tables;
        // convert num_tables to a string
        //char * filename = table_name;
        //char path[100];


        /*
        strcpy(path, db_loc);
        strcat(path, "/tables/");


        //create a new file in the tables directory
        char * filepath = strcat(path, filename);

        FILE *fp = fopen(filepath, "wb");


        char zero = '0';
        fwrite(&zero, sizeof(int), 1, fp);
        // add the table name to the schema
    */
        if(schema->num_tables == schema->max_num_tables){
            schema->max_num_tables *= 2;
            schema->tables = realloc(schema->tables, schema->max_num_tables * sizeof(char *));
        }


        schema->tables[num_tables] = *table_ptr;


        // close the file
        //fclose(fp);
        schema->num_tables++;

    }
    /*
  if(!has_primary_key){
      printf("No primary key defined\n");
      return false;
  }

  while (1) {


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
    table_ptr->num_attributes++;
  }

  // check has primary key
  if (!has_primary_key) {
    printf("No primary key defined\n");
    return false;
  }

  print_table_metadata(table_ptr);
  return true;
     */
    return true;
}

bool parse_create_table(char * command, char * db_loc, Schema * schema){

    char table_name[256];
    char attribute_name[256];
    char attribute_type[256];
    int num_attributes = 0;
    char * cmd = strtok(command, "(");
    char first_part[256];
    char second_part[256];

    strcpy(first_part, cmd);
    cmd = strtok(NULL, ")");
    strcpy(second_part, cmd);

    char * tokens = strtok(first_part, " ");

    tokens = strtok(NULL, " ");

    tokens = strtok(NULL, " ");

    strcpy(table_name, tokens);
    tokens = strtok(second_part, ",");
    char ** attribute_strings = malloc(sizeof(char *) * 256);
    while(tokens != NULL){
        attribute_strings[num_attributes] = tokens;
        tokens = strtok(NULL, ",");
        num_attributes++;
    }
    char * last_attr = attribute_strings[num_attributes - 1];
    last_attr[strcspn(last_attr, ")")] = '\0';
    return process_create_table(table_name, num_attributes, attribute_strings, db_loc, schema);
}

struct table get_table(char * table_name)
{
    struct attribute id = { "Student ID", INTEGER, NULL, 1 };
    struct attribute name = { "Student Name", VARCHAR, 5, 0 };

    struct attribute student_attributes[2];
    student_attributes[0] = id;
    student_attributes[1] = name;
    struct table student = { "Student", student_attributes, 2 };
    
    return student;
}

bool process_insert_record(char *command, char * db_loc, Schema * schema)
{
    // Add a semi-colon at the end of the command
    int command_len = strlen(command);
    command[command_len] = ';';
    command[command_len+1] = '\0';
    
    // Parse the table name and values out of the command
    char table_name[20], values[50];
    sscanf(command, "insert into %s values %[^;]", table_name, values);
    
    // Make sure the values are comma delimited with no space
    char values_delimited[50];
    values_delimited[0] = values[0];
    char prev = values[0];
    int j = 1;
    for(int i=1; i < strlen(values); i++)
    {
        if(!(prev == ',' && values[i] == ' '))
        {
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
    for(int i = 0; i < strlen(values_delimited); i++)
    {
        if(values_delimited[i] == ',')  
        {
            num_values++;
        }
    }
    num_values++;

    // Create the array to be returned
    struct table command_table = get_table(table_name);
    char *values_parsed[num_values][command_table.num_attributes];
    
    // Iterate through values
    char* tuple = strtok(values_delimited, ",");
    printf(tuple);
    printf("\n");
    while (tuple != NULL) 
    {
        tuple = strtok(NULL, ",");
        printf(tuple);
        printf("\n");
    }
}

void display_attributes(Schema * schema){

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

bool select_all(char *table_name, char *db_loc, Schema * schema) {
    char * filename = strcat(db_loc, table_name);
    FILE *fp = fopen(filename, "rb");
    if(fp == NULL) {
        printf("No such table %s\n", table_name);
        return false;
    }

    // TODO: Use the design shown on the writeup

    display_attributes(schema);

    int * page_locations = schema->page_locations;
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

bool parse_select(char * command, char * db_loc, Schema * schema) {
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

bool process_display_schema(char * command, char * db_loc, Schema * schema){
    printf("Display Schema not implemented!");
}

bool process_display_info(char * command, char * db_loc, Schema * schema) {
    printf("Display Info not implemented!");
}

void shut_down_database(){
    return;
}


void purge_page_buffer(){

}

void save_catalog(Schema * schema, char * db_loc){
    char path[100];
    strcpy(path, db_loc);
    strcat(path, "/catalog");
    FILE * fp = fopen(path, "wb");
    fwrite(&(*schema), sizeof(Schema), 1, fp);
}

void parse_command(char * command, char * db_loc, Schema * schema)
{
    // Self explanatory code.
    if(startsWith(command, "select")){
        parse_select(command, db_loc, schema);
    }
    else if(startsWith(command, "create")){
        parse_create_table(command, db_loc, schema);
    }
    else if(startsWith(command, "insert")){
        process_insert_record(command, db_loc, schema);
    }
    else if(startsWith(command, "display schema")){
        process_display_schema(command, db_loc, schema);
    }
    else if(startsWith(command, "display info")){
        process_display_info(command, db_loc, schema);
    }
    else{
        printf("Invalid command\n");
    }
    printf("\n");
}

void process(char * db_loc, Schema * schema){
    // Continuously accept commands from a user
    while(1){
        // Iterate through characters typed by the user
        // Stop iterating if the user enters <quit> or a semi-colon
        printf(">");
        char command[500];
        command[0] = '\0';
        char next_char = '\0';
        int command_idx = 0;
        while(((next_char = getchar()) != ';')){
            command[command_idx] = next_char;
            command_idx++;
            command[command_idx] = '\0';
            if(strcmp(command, "<quit>") == 0){
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
        while (strcspn(command, "\n") != strlen(command)){
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

void query_loop(char * db_loc, Schema * schema) { process(db_loc, schema); }

#endif