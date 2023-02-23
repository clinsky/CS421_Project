#ifndef PARSE_UTILS_H
#define PARSE_UTILS_H

#include "attribute_types.h"
#include <stdbool.h>
#include <string.h>

/*
 * Helper function that checks if str A has prefix B
 * Used to check for varchar( and char(
 * */
bool startsWith(const char *, const char *);

/*
 * Helper function that checks if str A has suffix B
 * Used to check for ) and );
 * */
bool endsWith(const char *, const char *);

char *attribute_type_to_string(ATTRIBUTE_TYPE t);

int attribute_type_to_int(ATTRIBUTE_TYPE t);

ATTRIBUTE_TYPE int_to_attribute_type(int t);

#endif
