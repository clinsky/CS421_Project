#ifndef ATTRIBUTE_TYPES_H
#define ATTRIBUTE_TYPES_H

#include <stdbool.h>

enum ATTRIBUTE_TYPE { INTEGER, BOOL, DOUBLE, CHAR, VARCHAR, INVALID_ATTR };
typedef enum ATTRIBUTE_TYPE ATTRIBUTE_TYPE;

struct attribute {
  char *name;          // name of attribute
  ATTRIBUTE_TYPE type; // type of attribute
  int len;             // len if char/varchar
  bool is_primary_key; // if is primarykey
  bool notnull; // if cannot be null
  bool unique; // if must be unique
};

typedef struct attribute Attribute;

struct attribute_values {
  ATTRIBUTE_TYPE type; // type of attribute
  int int_val;
  char *chars_val; // chars and varchars vals;
  double double_val;
  int bool_val;
  bool is_null;
};

typedef struct attribute_values Attribute_Values;

#endif
