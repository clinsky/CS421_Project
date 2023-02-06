#ifndef QUERY_PROCESSOR_H
#define QUERY_PROCESSOR_H

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
void process();

// parses a create table command from stdin
bool process_create_table();

// parses an insert into command from stdin
bool process_insert_record();

// parse a select command from stdin
bool process_select();

#endif
