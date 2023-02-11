#ifndef ATTRIBUTE_TYPES_H
#define ATTRIBUTE_TYPES_H

enum ATTRIBUTE_TYPE { INTEGER, BOOL, DOUBLE, CHAR, VARCHAR, INVALID_ATTR };
typedef enum ATTRIBUTE_TYPE ATTRIBUTE_TYPE;

struct attribute {
  char *name;          // name of attribute
  ATTRIBUTE_TYPE type; // type of attribute
  int len;             // len if char/varchar
  bool is_primary_key; // if is primarykey
};

typedef struct attribute Attribute;

#endif
