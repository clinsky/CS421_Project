#ifndef PARSE_UTILS_H
#define PARSE_UTILS_H

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

#endif
