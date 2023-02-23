#ifndef QUERY_PROCESSOR_H
#define QUERY_PROCESSOR_H

#include "attribute_types.h"
#include "display.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// defaults for now
#define MAX_NAME_LEN 100
#define MAX_ATTR_LEN 100
#define MAX_ATTRS 100

// main loop that reads from stdin
void query_loop();

// looks at first word from stdin
// delegates to either process_create_table,
// process_insert_record or process_select
void process(char * db_loc, Schema * schema);

// parses a create table command from stdin
bool process_create_table(char * command, char * db_loc, Schema * schema);

// parses an insert into command from stdin
bool process_insert_record(char * command, char * db_loc, Schema * schema);

// parse a select command from stdin
bool process_select();

// prints "SUCCESS" or "ERROR" depending on if statement was executed
void print_command_result(bool success);

// parse attribute type from string input and set it in attribute ptr
ATTRIBUTE_TYPE parse_attribute_type(char *, Attribute *);

#endif
