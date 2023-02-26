#include "page.h"

Record *check_valid_parsed_tuple(Table *table, char (*tuple_parsed)[50]) {
  Record *record = malloc(sizeof(Record));
  record->bitmap = 0;

  Attribute_Values *values =
      malloc(sizeof(Attribute_Values) * table->num_attributes);

  for (int i = 0; i < table->num_attributes; i++) {
    char *v = tuple_parsed[i];
    // printf("converting %s\n", v);
    ATTRIBUTE_TYPE type = table->attributes[i].type;
    values[i].type = type;
    if (strcmp(v, "null") == 0) {
      if (table->attributes[i].is_primary_key) {
        return NULL;
      }
      values[i].is_null = true;
      continue;
    }
    if (type == INTEGER) {
      // printf("trying to convert %s to integer..\n", v);
      // printf("%lu\n", strlen(v));
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
        // printf("The double value of '%s' is %lf\n", v, double_val);
      }
      values[i].double_val = double_val;
    } else if (type == BOOL) {
      int intval = atoi(v);
      if (intval == 0) {
        if (strcmp(v, "0") != 0) {
          return false;
        }
        values[i].bool_val = 0;
      } else {
        values[i].bool_val = 1;
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
    record->bitmap |= (1 << i);

    // store which attr # is the primary key
    if (table->attributes[i].is_primary_key) {
      record->primary_key_index = i;
    }
  }
  record->attr_vals = values;
  record->size = calculate_record_size(table, record);

  return record;
}

Page *read_page_from_file(Schema *schema, Table *table, char *file_path) {
  printf("reading page from file..\n");
  FILE *fp = fopen(file_path, "rb");
  Page *first_page = NULL;
  Page *prev_page = NULL;
  int x = 0;
  bool still_go = true;
  while (still_go) {
    int RELATIVE_START = x * schema->page_size;
    // printf("read page relative start: %d\n", RELATIVE_START);
    fseek(fp, RELATIVE_START, SEEK_SET);
    Page *page = malloc(sizeof(Page));
    page->max_size = schema->page_size;
    int has_next_page;
    fread(&has_next_page, sizeof(int), 1, fp);
    if (has_next_page != 1) {
      still_go = false;
    } else {
      page->next_page = malloc(sizeof(Page));
    }
    printf("has_next_page:%d\n", has_next_page);
    fread(&page->num_records, sizeof(int), 1, fp);
    printf("num records on  page %d: %d\n", x, page->num_records);
    page->records = malloc(sizeof(Record) * page->num_records);
    for (int record_num = 0; record_num < page->num_records; record_num++) {
      // read offset for the x+1th record
      Record *record = malloc(sizeof(Record));
      fseek(fp, RELATIVE_START + 4 + (4 * (record_num + 1)), SEEK_SET);
      int offset;
      fread(&offset, sizeof(int), 1, fp);
      printf("offset is: %d\n", offset);
      fseek(fp, RELATIVE_START + offset, SEEK_SET);
      printf("at pos %ld\n", ftell(fp));
      fread(&record->bitmap, sizeof(int), 1, fp);
      printf("bitmap read is %d\n", record->bitmap);

      record->attr_vals =
          malloc(sizeof(Attribute_Values) * table->num_attributes);

      for (int i = 0; i < table->num_attributes; i++) {
        Attribute_Values *curr_attr = &record->attr_vals[i];
        ATTRIBUTE_TYPE type = table->attributes[i].type;
        if (table->attributes[i].is_primary_key) {
          record->primary_key_index = i;
        }
        curr_attr->type = type;
        if (type == INTEGER) {
          // printf("reading int at pos %li\n", ftell(fp));
          fread(&curr_attr->int_val, sizeof(int), 1, fp);
          // printf("int was %d\n", curr_attr->int_val);
        } else if (type == DOUBLE) {
          fread(&curr_attr->double_val, sizeof(double), 1, fp);
          // printf("double was %f\n", curr_attr->double_val);
        } else if (type == BOOL) {
          fread(&curr_attr->bool_val, sizeof(int), 1, fp);
          // printf("bool was %d\n", curr_attr->bool_val);
        } else if (type == CHAR) {
          curr_attr->chars_val =
              malloc(sizeof(char) * table->attributes[i].len);
          fread(&curr_attr->chars_val, table->attributes[i].len,
                table->attributes[i].len, fp);
          // printf("char was %s\n", curr_attr->chars_val);
        } else if (type == VARCHAR) {
          int var_len;
          fread(&var_len, sizeof(int), 1, fp);
          curr_attr->chars_val = malloc(sizeof(char) * var_len);
          fread(&curr_attr->chars_val, sizeof(char), var_len, fp);
        }
      }
      record->size = calculate_record_size(table, record);
      page->records[record_num] = *record;
    }
    x += 1;
    if (first_page == NULL) {
      first_page = page;
    }
    if (prev_page != NULL) {
      prev_page->next_page = page;
    }
    prev_page = page;
  }

  fclose(fp);
  return first_page;
};

void write_page_to_file(Table *table, Page *p, char *file_path) {
  int RELATIVE_START = p->page_number * p->max_size;
  // printf("write page relative start: %d\n", RELATIVE_START);
  FILE *fp = fopen(file_path, "r+"); // read write mode without overwriting
  if (fp == NULL) {
    printf("Error opening %s file\n", file_path);
    return;
  }
  fseek(fp, RELATIVE_START, SEEK_SET);
  if (p->next_page != NULL) {
    int np = 1;
    fwrite(&np, sizeof(int), 1, fp);
  } else {
    int np = 0;
    fwrite(&np, sizeof(int), 1, fp);
  }
  // printf("%d is num recs\n", p->num_records);
  if (fwrite(&p->num_records, sizeof(int), 1, fp) != 1) {
    printf("wtf didn't write to file\n");
  } else {
    // printf("wrote num recs to file\n");
  }

  int prev = p->max_size;

  // write records
  for (int i = 0; i < p->num_records; i++) {
    Record *r = &p->records[i];
    int place = prev - (r->size);
    // print_record(table, r);
    // printf("record #%d size: %d\n", i, r->size);
    // printf("place #%d : %d\n", i, place);
    // 4 for # records, 4 for each offset
    fseek(fp, RELATIVE_START + 4 + (4 * (i + 1)), SEEK_SET);
    fwrite(&place, sizeof(int), 1, fp);
    fseek(fp, RELATIVE_START + place, SEEK_SET);
    // write bitmap
    // printf("at pos %li\n", ftell(fp));
    // printf("writing bitmap to file: %d..\n", p->records[i].bitmap);
    if (fwrite(&(p->records[i].bitmap), sizeof(int), 1, fp) != 1) {
      // printf("bitmap wasn't 1 for somereason\n");
    }
    // // write each attribute
    for (int j = 0; j < table->num_attributes; j++) {
      ATTRIBUTE_TYPE type = table->attributes[j].type;
      // write default values
      if (r->attr_vals[j].is_null) {
        printf("writing defaults for attr #:%d\n", j);
        if (type == INTEGER) {
          int default_int = 0;
          fwrite(&default_int, sizeof(int), 1, fp);
        } else if (type == DOUBLE) {
          double default_double = 0;
          fwrite(&default_double, sizeof(double), 1, fp);
        } else if (type == BOOL) {
          int default_bool = 0;
          fwrite(&default_bool, sizeof(int), 1, fp);
        } else if (type == CHAR) {
          int len = table->attributes[j].len;
          char *default_chars = malloc(len + 1);
          memset(default_chars, '@', len); // fill the string with '@'
          default_chars[len] = '\0';
          fwrite(&default_chars, sizeof(char), len, fp);
        } else if (type == VARCHAR) {
          // one byte '@' indicating null
          char *default_chars = malloc(2);
          memset(default_chars, '@', 2);
          default_chars[1] = '\0';
          fwrite(&default_chars, sizeof(char), 1, fp);
        }
      } else {
        if (type == INTEGER) {
          if (fwrite(&p->records[i].attr_vals[j].int_val, sizeof(int), 1, fp) !=
              1) {
            printf("failed writing int\n");
          } else {
          }
        } else if (type == DOUBLE) {
          fwrite(&p->records[i].attr_vals[j].double_val, sizeof(double), 1, fp);
        } else if (type == BOOL) {
          fwrite(&p->records[i].attr_vals[j].bool_val, sizeof(double), 1, fp);
        } else if (type == CHAR) {
          // fixed chars to read for CHAR type
          fwrite(&p->records[i].attr_vals[j].chars_val, sizeof(char),
                 table->attributes[j].len, fp);
        } else if (type == VARCHAR) {
          // write how many cahrs to read for varchar
          int num_chars = strlen(p->records[i].attr_vals[j].chars_val);
          // printf("writing var_len: %d\n", num_chars);
          fwrite(&num_chars, sizeof(int), 1, fp);
          if (fwrite(&p->records[i].attr_vals[j].chars_val, sizeof(char),
                     num_chars, fp) != num_chars) {
            printf("error writing var_len: %d\n", num_chars);
          }
        }
      }
      // update end pointer
      prev = place;
    }
  }
  fclose(fp);
}

Page *add_record_to_page(Schema *schema, Table *table, Record *record) {
  char filepath[100];
  snprintf(filepath, sizeof(filepath), "%s/%s", schema->db_path, table->name);
  Page *p;

  if (access(filepath, F_OK) != -1) {
    // printf("File %s exists\n", filepath);
    p = read_page_from_file(schema, table, filepath);
    if (p != NULL) {
      printf("successfully read page..\n");
    }
    p = insert_record_to_page(schema, table, p, record);
    print_page(table, p);
    write_page_to_file(table, p, filepath);
  } else {
    // printf("File %s does not exist\n", filepath);
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
    first_page->next_page = NULL;
    first_page->max_size = schema->page_size;
    bool enough_space_to_insert = check_enough_space(table, first_page, record);
    if (!enough_space_to_insert) {
      return NULL;
    }
    first_page->num_records = 1;
    first_page->record_capacity = 20;
    first_page->page_number = 0;
    first_page->offsets = malloc(sizeof(Offset) * first_page->record_capacity);
    first_page->records = malloc(sizeof(Record) * first_page->record_capacity);
    first_page->records[0] = *record;
    first_page->total_bytes_from_records += record->size;
    p = first_page;

    write_page_to_file(table, p, filepath);

    // read_page_from_file(schema, table, filepath);
  }
  return p;
}

Page *insert_record_to_page(Schema *schema, Table *table, Page *p,
                            Record *record) {
  printf("in insert_record_to_page, num records:%d\n", p->num_records);
  int pkey = record->primary_key_index;
  ATTRIBUTE_TYPE type = table->attributes[pkey].type;
  Page *curr_page = p;
  while (curr_page != NULL) {
    for (int i = 0; i < curr_page->num_records; i++) {
      printf("checking record #%d\n", i);
      Record *curr_record = &curr_page->records[i];
      printf("passeed page record\n");
      bool greater = 0;
      if (type == INTEGER) {
        printf("at %d val is %d\n", i, curr_record->attr_vals[pkey].int_val);
        greater = curr_record->attr_vals[pkey].int_val <
                  record->attr_vals[pkey].int_val;
      } else if (type == DOUBLE) {
        greater = curr_record->attr_vals[pkey].double_val <
                  record->attr_vals[pkey].double_val;
      } else if (type == BOOL) {
        greater = curr_record->attr_vals[pkey].bool_val <
                  record->attr_vals[pkey].bool_val;
      } else if (type == CHAR) {
        greater = strcmp(curr_record->attr_vals[pkey].chars_val,
                         record->attr_vals[pkey].chars_val) < 0;
      } else if (type == VARCHAR) {
        greater = strcmp(curr_record->attr_vals[pkey].chars_val,
                         record->attr_vals[pkey].chars_val) < 0;
      }
      if (!greater) {
        printf("was NOT greater than this record at %d\n", i);
        printf("curr num records: %d\n", curr_page->num_records);
        printf("curr num records capacity: %d\n", curr_page->record_capacity);
        if (curr_page->num_records + 1 > curr_page->record_capacity) {
          printf("reallocing..\n");
          curr_page->records =
              realloc(curr_page->records,
                      sizeof(Record) * (curr_page->num_records * 2));
        }
        curr_page->num_records += 1;
        for (int j = curr_page->num_records - 1; j > i; j--) {
          // print_record(table, &curr_page->records[j - 1]);
          curr_page->records[j] = curr_page->records[j - 1];
        }
        curr_page->records[i] = *record;
        return curr_page;
      } else {
        printf("was greater than this record\n");
      }
    }
    curr_page = curr_page->next_page;
  }
  printf("inserting a new page..\n");
  return p;
}

void print_page(Table *table, Page *p) {
  printf("PRINTING PAGE!!!\n");
  Page *curr_page = p;
  for (int k = 0; k < curr_page->num_records; k++) {
    printf("record #%d: ", k);
    for (int l = 0; l < table->num_attributes; l++) {
      ATTRIBUTE_TYPE type = curr_page->records[k].attr_vals[l].type;
      if (type == INTEGER) {
        printf("%d ", curr_page->records[k].attr_vals[l].int_val);
      } else if (type == DOUBLE) {
        printf("%f ", curr_page->records[k].attr_vals[l].double_val);
      } else if (type == BOOL) {
        printf("%d ", curr_page->records[k].attr_vals[l].bool_val);
      } else if (type == CHAR) {
        printf("%s ", curr_page->records[k].attr_vals[l].chars_val);
      } else if (type == VARCHAR) {
        printf("%s ", curr_page->records[k].attr_vals[l].chars_val);
      }
    }
    printf("\n");
  }
  printf("DONE PRINTING PAGE!!!\n");
}

void print_record(Table *table, Record *record) {
  printf("printing record: ");
  for (int l = 0; l < table->num_attributes; l++) {
    ATTRIBUTE_TYPE type = record->attr_vals[l].type;
    if (type == INTEGER) {
      printf("%d ", record->attr_vals[l].int_val);
    } else if (type == DOUBLE) {
      printf("%f ", record->attr_vals[l].double_val);
    } else if (type == BOOL) {
      printf("%d ", record->attr_vals[l].bool_val);
    } else if (type == CHAR) {
      printf("%s ", record->attr_vals[l].chars_val);
    } else if (type == VARCHAR) {
      printf("%s", record->attr_vals[l].chars_val);
    }
  }
  printf("\n");
}

int calculate_record_size(Table *table, Record *record) {
  int record_size = 4; // initialize with bitmap size
  for (int i = 0; i < table->num_attributes; i++) {
    ATTRIBUTE_TYPE type = table->attributes[i].type;
    if (type == INTEGER) {
      record_size += 4;
    } else if (type == DOUBLE) {
      record_size += 8;
    } else if (type == BOOL) {
      record_size += 4;
    } else if (type == CHAR) {
      // a char(x) is x size even if input is less than x
      record_size += table->attributes[i].len;
    } else if (type == VARCHAR) {
      if (record->attr_vals[i].is_null) {
        record_size += 4;
      } else {
        record_size += 4;
        record_size += strlen(record->attr_vals[i].chars_val);
      }
    }
  }
  // printf("record bitmap: ");
  int x = record->bitmap;
  for (int i = 0; i < table->num_attributes; i++) {
    if ((x & (1 << i)) != 0) {
      // printf("1");
    } else {
      // printf("0");
    }
  }
  // printf("\n");
  return record_size;
}

// enough space in the page?
bool check_enough_space(Table *table, Page *p, Record *record) {
  int check_next_page = 4;  // byte to determine if more pages next
  int num_record_space = 4; // first byte of page
  int offsets = (4 * (p->num_records + 1));
  int so_far = check_next_page + num_record_space + offsets +
               p->total_bytes_from_records;
  int record_size = record->size;
  int total_if_add = so_far + record_size;
  printf("record to be added is size: %d\n", record_size);
  printf("after adding this record, the page will have size: %d\n",
         total_if_add);
  if (total_if_add > p->max_size) {
    return false;
  }
  return true;
}
