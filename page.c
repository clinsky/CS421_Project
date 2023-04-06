#include "page.h"

Record *check_valid_parsed_tuple(Table *table, char **tuple_parsed) {
  Record *record = malloc(sizeof(Record));
  record->bitmap = 0;
  record->unique_attribute_indices =
      malloc(sizeof(int) * table->num_unique_attributes);

  Attribute_Values *values =
      malloc(sizeof(Attribute_Values) * table->num_attributes);
  int unique_attribute_count = 0;
  for (int i = 0; i < table->num_attributes; i++) {
    char *v = tuple_parsed[i];
    ATTRIBUTE_TYPE type = table->attributes[i].type;
    values[i].type = type;
    if (v == NULL) {
      if (table->attributes[i].is_primary_key) {
        printf("Primary key is null\n");
        printf("ERROR\n");
        return NULL;
      }
      values[i].is_null = true;
      // printf("Attribute index %d is null!\n", i);
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
      // printf("%d was intval..\n", intval);
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
      if (strcmp(v, "false") == 0) {
        values[i].bool_val = 0;
      } else if (strcmp(v, "true") == 0) {
        values[i].bool_val = 1;
      } else {
        printf("bool value should either be 'true' or 'false'\n");
        return false;
      }
    } else if (type == CHAR) {
      if (strlen(v) > table->attributes[i].len) {
        printf("%s can only accept %d chars\n", table->attributes[i].name,
               table->attributes[i].len);
        return false;
      }
      values[i].chars_val = malloc(table->attributes[i].len + 1);
      strncpy(values[i].chars_val, v, strlen(v));
      values[i].chars_val[table->attributes[i].len] = '\0';

    } else if (type == VARCHAR) {
      if (strlen(v) > table->attributes[i].len) {
        printf("%s can only accept %d chars\n", table->attributes[i].name,
               table->attributes[i].len);
        return false;
      }
      values[i].chars_val = malloc(strlen(v) + 1);
      strncpy(values[i].chars_val, v, strlen(v));
      values[i].chars_val[strlen(v)] = '\0';
    }
    record->bitmap |= (1 << i);

    // store which attr # is the primary key
    if (table->attributes[i].is_primary_key) {
      record->primary_key_index = i;
    } else if (table->attributes[i].unique) {
      record->unique_attribute_indices[unique_attribute_count] = i;
      unique_attribute_count++;
    }
  }
  record->attr_vals = values;
  record->size = calculate_record_size(table, record);

  return record;
}

Page *read_page_from_file(Schema *schema, Table *table, char *file_path) {
  if (access(file_path, F_OK) != 0) {
    // file for pages doesn't exist yet
    // printf("%s doesn't exist yet\n", file_path);
    return NULL;
  }
  // printf("reading page from file..\n");
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
    page->total_bytes_from_records = 0;
    page->record_capacity = 20;
    int has_next_page;
    // printf("reading has_next_page at pos %lu\n", ftell(fp));
    fread(&has_next_page, sizeof(int), 1, fp);
    // printf("has_next_page was: %d\n", has_next_page);
    if (has_next_page != 1) {
      still_go = false;
    } else {
      page->next_page = malloc(sizeof(Page));
    }
    // printf("has_next_page:%d\n", has_next_page);
    // printf("reading num records at %lu\n", ftell(fp));
    fread(&page->num_records, sizeof(int), 1, fp);
    // printf("num records: %d\n", page->num_records);
    // printf("num records on  page %d: %d\n", x, page->num_records);
    page->records = malloc(sizeof(Record) * page->num_records);
    for (int record_num = 0; record_num < page->num_records; record_num++) {
      // read offset for the x+1th record
      Record *record = malloc(sizeof(Record));
      fseek(fp, RELATIVE_START + 4 + (4 * (record_num + 1)), SEEK_SET);
      // printf("fp before offset: %d\n",
      //        RELATIVE_START + 4 + (4 * (record_num + 1)));
      int offset = 0;
      if (fread(&offset, sizeof(int), 1, fp) != 1) {
        // printf("failed to read byte for offset\n");
      }
      // offset = 11;
      // printf("offset is: %d\n", offset);
      fseek(fp, RELATIVE_START + offset, SEEK_SET);
      // printf("at pos %ld\n", ftell(fp));
      fread(&record->bitmap, sizeof(int), 1, fp);
      // printf("bitmap read is %d\n", record->bitmap);

      record->attr_vals =
          malloc(sizeof(Attribute_Values) * table->num_attributes);

      for (int i = 0; i < table->num_attributes; i++) {
        Attribute_Values *curr_attr = &record->attr_vals[i];
        if ((record->bitmap & (1 << i)) == 0) {
          // printf("record# %d attr %d is null\n", record_num, i);
          // printf("attr %d is null\n", i);
          curr_attr->is_null = true;
        }
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
              malloc(sizeof(char) * table->attributes[i].len + 1);
          // printf("about to read chars at %lu\n", ftell(fp));
          int charsread = fread(curr_attr->chars_val, sizeof(char),
                                table->attributes[i].len, fp);
          curr_attr->chars_val[table->attributes[i].len] = '\0';
          if (charsread != table->attributes[i].len) {
            // printf("failed to read %d chars, read %d instead\n",
            //        table->attributes[i].len, charsread);
          } else {
            // printf("%s was read with %d chars of len %lu\n",
            //        curr_attr->chars_val, charsread,
            //        strlen(curr_attr->chars_val));
          }
          // printf("char was %s\n", curr_attr->chars_val);
        } else if (type == VARCHAR) {
          int var_len;
          // printf("reading var_len at pos %li\n", ftell(fp));
          fread(&var_len, sizeof(int), 1, fp);
          curr_attr->chars_val = malloc(sizeof(char) * var_len + 1);
          // printf("reading varchar at pos %li, var_len was: %d\n", ftell(fp),
          //        var_len);
          fread(curr_attr->chars_val, sizeof(char), var_len, fp);
          curr_attr->chars_val[var_len] = '\0';
          // printf("char was %s\n", curr_attr->chars_val);
        }
      }
      record->size = calculate_record_size(table, record);
      page->total_bytes_from_records += record->size;
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
  FILE *fp = fopen(file_path, "wb+"); // read write mode without overwriting
  int page_count = 0;
  Page *temp = p;
  while (temp != NULL) {
    page_count += 1;
    if (temp->next_page != NULL) {
      temp = temp->next_page;
    } else {
      break;
    }
  }
  int MAX_SIZE = p->max_size;
  // printf("page count was %d\n", page_count);
  fseek(fp, (p->max_size * page_count) - 1, SEEK_SET);
  fputc(0, fp);

  for (int a = 0; a < page_count; a++) {
    int RELATIVE_START = a * p->max_size;
    // printf("write page %d relative start: %d\n", a, RELATIVE_START);
    if (fp == NULL) {
      printf("Error opening %s file\n", file_path);
      printf("ERROR\n");
      return;
    }
    fseek(fp, RELATIVE_START, SEEK_SET);
    if (p->next_page != NULL) {
      int np = 1;
      fwrite(&np, sizeof(int), 1, fp);
    } else {
      int np = 37;
      fwrite(&np, sizeof(int), 1, fp);
    }
    // printf("%d is num recs\n", p->num_records);
    // printf("writing num recs at pos %lu\n", ftell(fp));
    if (fwrite(&p->num_records, sizeof(int), 1, fp) != 1) {
      // printf("wtf didn't write to file\n");
    } else {
      // printf("wrote num recs to file\n");
    }

    int prev = MAX_SIZE;

    // write records
    for (int i = 0; i < p->num_records; i++) {
      Record *r = &p->records[i];
      int place = prev - (r->size);
      // print_record(table, r);
      // printf("record #%d size: %d\n", i, r->size);
      // printf("place #%d : %d\n", i, place);
      // 4 for # records, 4 for each offset
      fseek(fp, RELATIVE_START + 4 + (4 * (i + 1)), SEEK_SET);
      // printf("writing place at %ld\n", ftell(fp));
      if (fwrite(&place, sizeof(int), 1, fp) != 1) {
        // printf("didnt write place for some reason\n");
      }
      fseek(fp, RELATIVE_START + place, SEEK_SET);
      // printf("writing record at pos %li\n", ftell(fp));

      // write bitmap
      // printf("at pos %li\n", ftell(fp));
      // printf("writing bitmap to file: %d..\n", p->records[i].bitmap);
      if (fwrite(&(p->records[i].bitmap), sizeof(int), 1, fp) != 1) {
        // printf("bitmap wasn't 1 for somereason\n");
      }
      // for (int b = 0; b < table->num_attributes; b++) {
      //   if ((p->records[i].bitmap & (1 << b)) != 0) {
      //     printf("1");
      //   } else {
      //     printf("0");
      //   }
      // }
      // printf("\n");
      // // write each attribute
      for (int j = 0; j < table->num_attributes; j++) {
        ATTRIBUTE_TYPE type = table->attributes[j].type;
        // write default values
        if (r->attr_vals[j].is_null) {
          // printf("writing defaults for attr #:%d\n", j);
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
            char *default_chars = malloc(sizeof(char) * len + 1);
            memset(default_chars, '@', len); // fill the string with '@'
            default_chars[len] = '\0';
            fwrite(&default_chars, sizeof(char), len, fp);
          } else if (type == VARCHAR) {
            int zero_len = 0;
            fwrite(&zero_len, sizeof(int), 1, fp);
          }
        } else {
          if (type == INTEGER) {
            // printf("at pos %li writing int %d\n", ftell(fp),
            //        p->records[i].attr_vals[j].int_val);
            if (fwrite(&p->records[i].attr_vals[j].int_val, sizeof(int), 1,
                       fp) != 1) {
              // printf("Failed writing int\n");
              // printf("ERROR\n");
            } else {
            }
          } else if (type == DOUBLE) {
            fwrite(&p->records[i].attr_vals[j].double_val, sizeof(double), 1,
                   fp);
          } else if (type == BOOL) {
            fwrite(&p->records[i].attr_vals[j].bool_val, sizeof(double), 1, fp);
          } else if (type == CHAR) {
            // printf("writing chars at pos %lu\n", ftell(fp));

            char *temp = malloc(sizeof(char) * table->attributes[j].len + 1);
            strncpy(temp, p->records[i].attr_vals[j].chars_val,
                    strlen(p->records[i].attr_vals[j].chars_val));
            for (int t = strlen(p->records[i].attr_vals[j].chars_val);
                 t < table->attributes[j].len; t++) {
              temp[t] = ' '; // padd with space
            }
            temp[table->attributes[j].len] = '\0';
            // printf("temp is %s with len %lu\n", temp, strlen(temp));
            // fixed chars to read for CHAR type
            int charswrite =
                fwrite(temp, sizeof(char), table->attributes[j].len, fp);
            // printf(
            //     "wrote %s with %d chars of len %lu, should of wrote %d
            //     chars\n", temp, charswrite, strlen(temp),
            //     table->attributes[j].len);
          } else if (type == VARCHAR) {
            // write how many cahrs to read for varchar
            int num_chars = strlen(p->records[i].attr_vals[j].chars_val);
            // printf("writing var_len: %d at pos %li\n", num_chars, ftell(fp));
            fwrite(&num_chars, sizeof(int), 1, fp);
            // printf("writing var char %s at: %li\n",
            // p->records[i].attr_vals[j].chars_val, ftell(fp));
            char *charsval = p->records[i].attr_vals[j].chars_val;
            if (fwrite(charsval, sizeof(char), num_chars, fp) != num_chars) {
              // printf("didn't write %d num chars\n", num_chars);
            } else {

              // printf("wrote %d num chars\n", num_chars);
            }

            // printf("just wrote chars of %s\n", charsval);
          }
        }
        // update end pointer
        prev = place;
      }
    }
    p = p->next_page;
  }
  fclose(fp);
}

Page *add_record_to_page(Schema *schema, Table *table, Record *record,
                         Bufferm *buffer) {
  char filepath[100];
  snprintf(filepath, sizeof(filepath), "%s/%s", schema->db_path, table->name);
  Page *p;

  if (access(filepath, F_OK) != -1) {
    // printf("File %s exists\n", filepath);
    p = find_in_buffer(buffer, table);
    if (p != NULL) {
      // printf("buffer had %s\n", table->name);
      p = insert_record_to_page(schema, table, p, record);
    } else {
      p = read_page_from_file(schema, table, filepath);
      p = insert_record_to_page(schema, table, p, record);
      if (p != NULL) {
        add_to_buffer(buffer, table, p, filepath);
      }
    }
    if (p != NULL) {
      // printf("successfully read page..\n");
    }
    // flush_buffer(buffer);
    return p;
  } else {
    // printf("File %s does not exist for adding record to page\n", filepath);
    // FILE *fp = fopen(filepath, "w");
    // if (fp != NULL) {
    //   // printf("File %s created successfully\n", filepath);
    //   fseek(fp, schema->page_size - 1, SEEK_SET);
    //   fputc(0, fp);
    //   fclose(fp);
    //
    // } else {
    //   printf("Error creating %s file\n", filepath);
    //   printf("ERROR\n");
    //   return NULL;
    // }
    p = find_in_buffer(buffer, table);
    if (p != NULL) {
      // printf("buffer had %s\n", table->name);
      p = insert_record_to_page(schema, table, p, record);
    } else {
      Page *first_page = malloc(sizeof(Page));
      first_page->next_page = NULL;
      first_page->max_size = schema->page_size;
      first_page->num_records = 1;
      first_page->record_capacity = 20;
      first_page->page_number = 0;
      first_page->offsets =
          malloc(sizeof(Offset) * first_page->record_capacity);
      first_page->records =
          malloc(sizeof(Record) * first_page->record_capacity);
      first_page->records[0] = *record;
      // printf("printing bitmap in add record\n");
      // for (int i = 0; i < table->num_attributes; i++) {
      //   if ((record->bitmap & (1 << i)) != 0) {
      //     printf("1");
      //   } else {
      //     printf("0");
      //   }
      // }
      // printf("\n");
      first_page->total_bytes_from_records += record->size;
      if (is_page_overfull(first_page)) {
        // printf("RECORD IS LARGER THAN PAGE SIZE...\n");
        return NULL;
      }
      p = first_page;
      write_page_to_file(table, p, filepath);
      // p = read_page_from_file(schema, table, filepath);
      add_to_buffer(buffer, table, p, filepath);
    }
    // print_page(table, p);
  }
  return p;
}

Page *insert_record_to_page(Schema *schema, Table *table, Page *p,
                            Record *record) {
  // printf("in insert_record_to_page, num records:%d %d\n", p->num_records,
  // record->size);
  int pkey = record->primary_key_index;
  ATTRIBUTE_TYPE type = table->attributes[pkey].type;
  Page *first_page = p;
  Page *curr_page = p;
  Page *prev_page = NULL;
  while (curr_page != NULL) {
    for (int i = 0; i < curr_page->num_records; i++) {
      // printf("checking record #%d\n", i);
      Record *curr_record = &curr_page->records[i];
      // printf("passeed page record\n");
      bool greater = 0;
      if (type == INTEGER) {
        if (curr_record->attr_vals[pkey].int_val ==
            record->attr_vals[pkey].int_val) {
          printf("Duplicate primary key %d\n", record->attr_vals[pkey].int_val);
          printf("ERROR\n");
          return NULL;
        }
        greater = curr_record->attr_vals[pkey].int_val <
                  record->attr_vals[pkey].int_val;
      } else if (type == DOUBLE) {
        if (curr_record->attr_vals[pkey].double_val ==
            record->attr_vals[pkey].double_val) {
          printf("Duplicate primary key %f\n",
                 record->attr_vals[pkey].double_val);
          printf("ERROR\n");
          return NULL;
        }
        greater = curr_record->attr_vals[pkey].double_val <
                  record->attr_vals[pkey].double_val;
      } else if (type == BOOL) {
        if (curr_record->attr_vals[pkey].bool_val ==
            record->attr_vals[pkey].bool_val) {
          printf("Duplicate primary key %d\n",
                 record->attr_vals[pkey].bool_val);
          printf("ERROR\n");
        }
        greater = curr_record->attr_vals[pkey].bool_val <
                  record->attr_vals[pkey].bool_val;
      } else if (type == CHAR) {
        if (strcmp(curr_record->attr_vals[pkey].chars_val,
                   record->attr_vals[pkey].chars_val) == 0) {
          printf("Duplicate primary key %s\n",
                 record->attr_vals[pkey].chars_val);
          printf("ERROR\n");
        }
        greater = strcmp(curr_record->attr_vals[pkey].chars_val,
                         record->attr_vals[pkey].chars_val) < 0;
      } else if (type == VARCHAR) {
        if (strcmp(curr_record->attr_vals[pkey].chars_val,
                   record->attr_vals[pkey].chars_val) == 0) {
          printf("Duplicate primary key %s\n",
                 record->attr_vals[pkey].chars_val);
          printf("ERROR\n");
        }
        greater = strcmp(curr_record->attr_vals[pkey].chars_val,
                         record->attr_vals[pkey].chars_val) < 0;
      }

      for (int j = 0; j < table->num_unique_attributes; j++) {
        int unique_attribute_index = record->unique_attribute_indices[j];
        ATTRIBUTE_TYPE unique_attribute_type =
            table->attributes[unique_attribute_index].type;
        if (unique_attribute_type == INTEGER) {
          if (curr_record->attr_vals[unique_attribute_index].int_val ==
              record->attr_vals[unique_attribute_index].int_val) {
            printf("Duplicate unique attribute %d\n",
                   record->attr_vals[unique_attribute_index].int_val);
            printf("ERROR\n");
            return NULL;
          }
        } else if (unique_attribute_type == DOUBLE) {
          if (curr_record->attr_vals[unique_attribute_index].double_val ==
              record->attr_vals[unique_attribute_index].double_val) {
            printf("Duplicate unique attribute %f\n",
                   record->attr_vals[unique_attribute_index].double_val);
            printf("ERROR\n");
            return NULL;
          }
        } else if (unique_attribute_type == BOOL) {
          if (curr_record->attr_vals[unique_attribute_index].bool_val ==
              record->attr_vals[unique_attribute_index].bool_val) {
            printf("Duplicate unique attribute %d\n",
                   record->attr_vals[unique_attribute_index].bool_val);
            printf("ERROR\n");
            return NULL;
          }
        } else if (unique_attribute_type == CHAR) {
          if (strcmp(curr_record->attr_vals[unique_attribute_index].chars_val,
                     record->attr_vals[unique_attribute_index].chars_val) ==
              0) {
            printf("Duplicate unique attribute %s\n",
                   record->attr_vals[unique_attribute_index].chars_val);
            printf("ERROR\n");
            return NULL;
          }
        } else if (unique_attribute_type == VARCHAR) {
          if (strcmp(curr_record->attr_vals[unique_attribute_index].chars_val,
                     record->attr_vals[unique_attribute_index].chars_val) ==
              0) {
            printf("Duplicate unique attribute %s\n",
                   record->attr_vals[unique_attribute_index].chars_val);
            printf("ERROR\n");
            return NULL;
          }
        }
      }

      if (!greater) {
        // printf("was NOT greater than this record at %d\n", i);
        // printf("curr num records: %d\n", curr_page->num_records);
        // printf("curr num records capacity: %d\n",
        // curr_page->record_capacity);
        if (curr_page->num_records + 1 > curr_page->record_capacity) {
          // printf("reallocing..\n");
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
        curr_page->total_bytes_from_records += record->size;
        make_new_page_if_full(curr_page);
        return first_page;
      } else {
      }
    }
    prev_page = curr_page;
    curr_page = curr_page->next_page;
  }

  // insert record into last page (aka prev page)
  // printf("last page num records: %d\n", prev_page->num_records);
  if (prev_page->num_records + 1 > prev_page->record_capacity) {
    // printf("reallocing..\n");
    prev_page->records = realloc(prev_page->records,
                                 sizeof(Record) * (prev_page->num_records * 2));
  }
  prev_page->num_records += 1;
  prev_page->records[prev_page->num_records - 1] = *record;
  prev_page->total_bytes_from_records += record->size;

  make_new_page_if_full(prev_page);

  return first_page;
}

void make_new_page_if_full(Page *prev_page) {
  if (is_page_overfull(prev_page)) {
    // printf("WAS OVERFULL...\n");
    Page *new_page = malloc(sizeof(Page));
    new_page->max_size = prev_page->max_size;
    new_page->page_number = prev_page->page_number + 1;
    if (prev_page->next_page != NULL) {
      new_page->next_page = prev_page->next_page;
    } else {
      new_page->next_page = NULL;
    }
    int half = prev_page->num_records / 2;
    int other_half = prev_page->num_records - half;
    new_page->records = malloc(sizeof(Record) * (other_half * 2));
    new_page->record_capacity = other_half * 2;
    new_page->num_records = other_half;
    prev_page->num_records = half;
    for (int i = 0; i < other_half; i++) {
      new_page->records[i] = prev_page->records[i + half];
    }
    prev_page->next_page = new_page;
  }
}

void print_page(Table *table, Page *p) {
  Page *curr_page = p;
  int page_num = 1;
  // printf("in print page..\n");
  while (curr_page != NULL) {
    // printf("printing page# %d with num records: %d\n", page_num,
    //        curr_page->num_records);
    for (int k = 0; k < curr_page->num_records; k++) {
      // printf("record #%d of size %d: \n", k, curr_page->records[k].size);
      printf("| ");
      for (int l = 0; l < table->num_attributes; l++) {
        ATTRIBUTE_TYPE type = curr_page->records[k].attr_vals[l].type;
        if (curr_page->records[k].attr_vals[l].is_null) {
          printf("null ");
          continue;
        }
        if (type == INTEGER) {
          printf("%d | ", curr_page->records[k].attr_vals[l].int_val);
        } else if (type == DOUBLE) {
          printf("%f | ", curr_page->records[k].attr_vals[l].double_val);
        } else if (type == BOOL) {
          printf("%d | ", curr_page->records[k].attr_vals[l].bool_val);
        } else if (type == CHAR) {
          printf("%s | ", curr_page->records[k].attr_vals[l].chars_val);
        } else if (type == VARCHAR) {
          printf("%s | ", curr_page->records[k].attr_vals[l].chars_val);
        }
      }
      printf("\n");
    }
    if (curr_page->next_page == NULL) {
      break;
    }
    curr_page = curr_page->next_page;
    page_num += 1;
  }
}

void print_page_where(Table *table, Page *p, ConditionalParseTree * conditionalParseTree) {
    Page *curr_page = p;
    int page_num = 1;
    // printf("in print page..\n");
    while (curr_page != NULL) {
        // printf("printing page# %d with num records: %d\n", page_num,
        //        curr_page->num_records);
        for (int k = 0; k < curr_page->num_records; k++) {
            // printf("record #%d of size %d: \n", k, curr_page->records[k].size);
            if(evaluateCondition(&(curr_page->records[k]), conditionalParseTree, table)) {
                printf("| ");
                for (int l = 0; l < table->num_attributes; l++) {
                    ATTRIBUTE_TYPE type = curr_page->records[k].attr_vals[l].type;
                    if (curr_page->records[k].attr_vals[l].is_null) {
                        printf("null ");
                        continue;
                    }
                    if (type == INTEGER) {
                        printf("%d | ", curr_page->records[k].attr_vals[l].int_val);
                    } else if (type == DOUBLE) {
                        printf("%f | ", curr_page->records[k].attr_vals[l].double_val);
                    } else if (type == BOOL) {
                        printf("%d | ", curr_page->records[k].attr_vals[l].bool_val);
                    } else if (type == CHAR) {
                        printf("%s | ", curr_page->records[k].attr_vals[l].chars_val);
                    } else if (type == VARCHAR) {
                        printf("%s | ", curr_page->records[k].attr_vals[l].chars_val);
                    }
                }
                printf("\n");
            }

        }
        if (curr_page->next_page == NULL) {
            break;
        }
        curr_page = curr_page->next_page;
        page_num += 1;
    }

}

void print_page_where_projection(Table *table, Page *p, ConditionalParseTree * conditionalParseTree, char ** requested_attributes, int num_attributes_requested){
    Page *curr_page = p;
    int page_num = 1;
    // printf("in print page..\n");
    while (curr_page != NULL) {
        // printf("printing page# %d with num records: %d\n", page_num,
        //        curr_page->num_records);
        for (int k = 0; k < curr_page->num_records; k++) {
            // printf("record #%d of size %d: \n", k, curr_page->records[k].size);
            if(evaluateCondition(&(curr_page->records[k]), conditionalParseTree, table)) {
                printf("| ");
                for (int i = 0; i < num_attributes_requested; i++) {

                    int l = 0;
                    for(int j = 0; j < table->num_attributes; j++){
                        if(strcmp(table->attributes[j].name, requested_attributes[i]) == 0){
                            l = j;
                        }
                    }

                    ATTRIBUTE_TYPE type = curr_page->records[k].attr_vals[l].type;
                    if (curr_page->records[k].attr_vals[l].is_null) {
                        printf("null ");
                        continue;
                    }
                    if (type == INTEGER) {
                        printf("%d | ", curr_page->records[k].attr_vals[l].int_val);
                    } else if (type == DOUBLE) {
                        printf("%f | ", curr_page->records[k].attr_vals[l].double_val);
                    } else if (type == BOOL) {
                        printf("%d | ", curr_page->records[k].attr_vals[l].bool_val);
                    } else if (type == CHAR) {
                        printf("%s | ", curr_page->records[k].attr_vals[l].chars_val);
                    } else if (type == VARCHAR) {
                        printf("%s | ", curr_page->records[k].attr_vals[l].chars_val);
                    }
                }
                printf("\n");
            }

        }
        if (curr_page->next_page == NULL) {
            break;
        }
        curr_page = curr_page->next_page;
        page_num += 1;
    }
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
      printf("%s ", record->attr_vals[l].chars_val);
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
  // printf("record to be added is size: %d\n", record_size);
  // printf("after adding this record, the page will have size: %d\n",
  // total_if_add);
  if (total_if_add > p->max_size) {
    return false;
  }
  return true;
}

bool is_page_overfull(Page *p) {
  int base = 4 + 4;
  int from_records = p->total_bytes_from_records;
  int for_offsets = p->num_records * 4;
  int total = base + from_records + for_offsets;
  // printf("total size is %d and max size is %d\n", total, p->max_size);
  return total > p->max_size;
}

Bufferm *create_new_bufferm(int max_pages) {
  Bufferm *b = malloc(sizeof(Bufferm));
  b->curr_pages = 0;
  b->max_pages = max_pages;
  b->entries = malloc(sizeof(Buffer_Entry) * max_pages);
  b->counter = 1;
  return b;
}

Page *find_in_buffer(Bufferm *b, Table *table) {
  b->counter += 1;
  for (int i = 0; i < b->curr_pages; i++) {
    if (strcmp(table->name, b->entries[i].table_name) == 0) {
      // printf("found page %s in buffer\n", table->name);
      b->entries[i].last_used = b->counter;
      return b->entries[i].page;
    }
  }
  return NULL;
}

void add_to_buffer(Bufferm *b, Table *table, Page *p, char *filepath) {
  b->counter += 1;
  Buffer_Entry *new_entry = malloc(sizeof(Buffer_Entry));
  if (b->curr_pages + 1 > b->max_pages) {
    // loop find  lowest last used;
    int max = 99999999;
    Buffer_Entry *to_remove = NULL;
    for (int i = 0; i < b->curr_pages; i++) {
      if (b->entries[i].last_used < max) {
        max = b->entries[i].last_used;
        to_remove = &b->entries[i];
      }
    }
    write_page_to_file(table, to_remove->page, to_remove->file_path);
  } else {
    // printf("page added to buffer pos:%d\n", b->curr_pages);
    new_entry->page = p;
    // printf("in add to buffer bitmap\n");
    // for (int i = 0; i < table->num_attributes; i++) {
    //   if ((p->records[0].bitmap & (1 << i)) != 0) {
    //     printf("1");
    //   } else {
    //     printf("0");
    //   }
    // }
    // printf("\n");
    new_entry->table_name = malloc(sizeof(char) * strlen(table->name));
    new_entry->file_path = malloc(sizeof(char) * strlen(filepath));
    new_entry->last_used = b->counter;
    new_entry->table = table;
    strcpy(new_entry->file_path, filepath);
    strcpy(new_entry->table_name, table->name);
    b->entries[b->curr_pages] = *new_entry;
    b->curr_pages += 1;
    // printf("adding %s %s to buffer..\n", new_entry->table_name,
    // new_entry->file_path);
  }
}

void flush_buffer(Bufferm *b) {
  b->counter += 1;
  for (int i = 0; i < b->curr_pages; b++) {
    Page *p = b->entries[i].page;
    // printf("printing page in flush buffer\n");
    // printf("curr page: %d\n", i);
    // printf("in flush buffer bitmap\n");
    // printf("%d\n", b->entries[i].table->num_attributes);
    // printf("num records in p: %d\n", p->num_records);
    // printf("%d\n", p->records[0].bitmap);
    if (b->entries[i].table == NULL) {
      // printf("table is null\n");
    } else {
      // printf("table not null\n");
      // printf("has %d attr\n", b->entries[i].table->num_attributes);
    }
    // for (int j = 0; j < b->entries[i].table->num_attributes; j++) {
    //   if ((p->records[0].bitmap & (1 << j)) != 0) {
    //     printf("1");
    //   } else {
    //     printf("0");
    //   }
    // }
    // printf("\n");
    // print_page(b->entries[i].table, p);
    write_page_to_file(b->entries[i].table, p, b->entries[i].file_path);
  }
}

Page *remove_from_buffer(Bufferm *b, Table *table) {
  // printf("in remove from buffer\n");
  b->counter += 1;
  // printf("buffer has %d pages\n", b->curr_pages);
  for (int i = 0; i < b->curr_pages; i++) {
    // printf("%s is table im looking for vs curr %s\n", table->name,
    //        b->entries[i].table_name);
    if (strcmp(table->name, b->entries[i].table_name) == 0) {
      Page *removed_page = b->entries[i].page;
      Buffer_Entry *last_entry = &b->entries[b->curr_pages - 1];
      b->entries[i] = *last_entry;
      // printf("found page %s in buffer\n", table->name);
      b->entries[i].last_used = b->counter;
      // printf("removed %s from buffer..\n", table->name);

      // dont actually "remove" from buffer.
      // on next buffer entry, it will be placed in last available spot
      b->curr_pages -= 1;
      return removed_page;
    }
  }
  // printf("%s wasn't in the buffer\n", table->name);
  return NULL;
}
