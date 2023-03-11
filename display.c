#include "display.h"

void print_table_metadata(Table *t) {
  printf("Table name: %s\n", t->name);
  printf("Table schema:\n");
  printf("Number of attributes: %d\n", t->num_attributes);
  for (int i = 0; i < t->num_attributes; i++) {
    if ((t->attributes[i]).type == CHAR || (t->attributes[i]).type == VARCHAR) {
      printf("\t%s:%s\t(%d) ", (t->attributes + i)->name,
             attribute_type_to_string((t->attributes + i)->type),
             (t->attributes[i]).len);

    } else {

      printf("\t%s:%s ", (t->attributes + i)->name,
             attribute_type_to_string((t->attributes[i]).type));
    }
    if (t->attributes[i].is_primary_key) {
      printf(" primarykey ");
    }
    if (t->attributes[i].notnull) {
      printf(" notnull ");
    }
    if (t->attributes[i].unique) {
      printf(" unique ");
    }
    printf("\n");
  }
}
