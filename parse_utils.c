#include "parse_utils.h"

bool startsWith(const char *A, const char *B) {
  return strncmp(A, B, strlen(B)) == 0;
}

bool endsWith(const char *str, const char *suffix) {
  if (!str || !suffix)
    return 0;
  size_t lenstr = strlen(str);
  size_t lensuffix = strlen(suffix);
  if (lensuffix > lenstr)
    return 0;
  return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

char *attribute_type_to_string(ATTRIBUTE_TYPE t) {
  if (t == INTEGER) {
    return "integer";
  } else if (t == BOOL) {
    return "bool";
  } else if (t == DOUBLE) {
    return "double";
  } else if (t == CHAR) {
    return "char";
  } else if (t == VARCHAR) {
    return "varchar";
  }
  return "";
}
