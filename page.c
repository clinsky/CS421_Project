#include "page.h"
#include "attribute_types.h"
#include "catalog.h"
#include <dirent.h>
#include <string.h>

Attribute_Values *check_valid_parsed_tuple(Table *table,
                                           char (*tuple_parsed)[50]) {
  Attribute_Values *values =
      malloc(sizeof(Attribute_Values) * table->num_attributes);

  for (int i = 0; i < table->num_attributes; i++) {
    char *v = tuple_parsed[i];
    printf("converting %s\n", v);
    ATTRIBUTE_TYPE type = table->attributes[i].type;
    if (strcmp(v, "null") == 0) {
      if (table->attributes[i].is_primary_key) {
        return NULL;
      }
      values[i].is_null = true;
    }
    if (type == INTEGER) {
      printf("trying to convert %s to integer..\n", v);
      printf("%lu\n", strlen(v));
      int intval = atoi(v);
      if (intval == 0) {
        if (strcmp(v, "0") != 0) {
          return NULL;
        }
      }
      values[i].int_val = intval;
    } else if (type == DOUBLE) {
      char *endptr;
      double double_val = strtod(v, &endptr);
      if (endptr == v) {
        return NULL;
      } else if (*endptr != '\0') {
        return NULL;
      } else {
        printf("The double value of '%s' is %lf\n", v, double_val);
      }
      values[i].double_val = double_val;
    } else if (type == BOOL) {
      int intval = atoi(v);
      if (intval == 0) {
        if (strcmp(v, "0") != 0) {
          return false;
        }
        values[i].bool_val = false;
      } else {
        values[i].bool_val = true;
      }
    } else if (type == CHAR) {
      if (strlen(v) > table->attributes[i].len) {
        return false;
      }
      values[i].chars_val = malloc(table->attributes[i].len + 1);
      strncpy(values[i].chars_val, v, strlen(v));
      values[i].chars_val[table->attributes[i].len] = '\0';

    } else if (type == VARCHAR) {
      if (strlen(v) > table->attributes[i].len) {
        return false;
      }
      values[i].chars_val = malloc(strlen(v));
      strncpy(values[i].chars_val, v, strlen(v));
    }
  }
  return values;
}

Page *read_page_from_file(Schema *schema, char *file_path) {
  FILE *fp = fopen(file_path, "rb");
  Page *first_page = malloc(sizeof(Page));
  first_page->max_size = schema->page_size;
  fwrite(&first_page->num_records, sizeof(int), 1, fp);
  return NULL;
};

void write_page_to_file(Table *table, Page *p, char *file_path) {
  FILE *fp = fopen(file_path, "r+"); // read write mode without overwriting
  if (fp == NULL) {
    printf("Error opening %s file\n", file_path);
    return;
  }
  fseek(fp, 0, SEEK_SET);
  fwrite(&p->num_records, sizeof(int), 1, fp);
}

Page *add_record_to_page(Schema *schema, Table *table,
                         Attribute_Values *attr_vals) {
  char filepath[100];
  snprintf(filepath, sizeof(filepath), "%s/%s", schema->db_path, table->name);
  Page *p;
  if (access(filepath, F_OK) != -1) {
    printf("File %s exists\n", filepath);
    p = read_page_from_file(schema, filepath);
  } else {
    printf("File %s does not exist\n", filepath);
    FILE *fp = fopen(filepath, "w");
    if (fp != NULL) {
      printf("File %s created successfully\n", filepath);
      fseek(fp, schema->page_size - 1, SEEK_SET);
      fputc(0, fp);
      fclose(fp);
    } else {
      printf("Error creating %s file\n", filepath);
      return NULL;
    }
    Page *first_page = malloc(sizeof(Page));
    first_page->max_size = schema->page_size;
    bool enough_space_to_insert =
        check_enough_space(table, first_page, attr_vals);
    if (!enough_space_to_insert) {
      return NULL;
    }
    p = first_page;
  }
  return p;
}

int calculate_record_size(Table *table, Attribute_Values *attr_vals) {
  int record_size = 0;
  for (int i = 0; i < table->num_attributes; i++) {
    ATTRIBUTE_TYPE type = table->attributes[i].type;
    if (type == INTEGER) {
      record_size += 4;
    } else if (type == DOUBLE) {
      record_size += 8;
    } else if (type == BOOL) {
      record_size += 2;
    } else if (type == CHAR) {
      // a char(x) is x size even if input is less than x
      record_size += table->attributes[i].len;
    } else if (type == VARCHAR) {
      record_size += strlen(attr_vals->chars_val);
    }
  }
  return record_size;
}

bool check_enough_space(Table *table, Page *p, Attribute_Values *attr_vals) {
  int num_record_space = 4; // first byte of page
  int offsets = (8 * (p->num_records + 1));
  int so_far = num_record_space + offsets + p->total_bytes_from_records;
  int record_size = calculate_record_size(table, attr_vals);
  int total_if_add = so_far + record_size;
  printf("record to be added is size: %d\n", record_size);
  printf("after adding this record, the page will have size: %d\n",
         total_if_add);
  if (total_if_add > p->max_size) {
    return false;
  }
  return true;
}
