#include "catalog.h"
#include "page.h"
#include "parse_utils.h"
#include "record.h"

Schema *create_schema(char *db_loc, int page_size, int buffer_size) {
  Schema *db_schema = read_catalog(db_loc); // this also allocs 100 tables
  strncpy(db_schema->db_path, db_loc, strlen(db_loc));
  db_schema->page_size = page_size;
  db_schema->buffer_size = buffer_size;
  db_schema->num_tables = 0;
  db_schema->max_num_tables = 10;
  return db_schema;
}

void increment_table_count(char *db_loc) {
  char filepath[100];
  snprintf(filepath, sizeof(filepath), "%s/%s", db_loc, "catalog");
  FILE *fp = fopen(filepath, "rb+"); // rb+ for reading and writing
  int table_count;
  if (fseek(fp, 0, SEEK_SET) != 0) {
    printf("failed to seek to beginning of file\n");
    fclose(fp);
    return;
  }

  // read #table count
  if (fread(&table_count, sizeof(int), 1, fp) != 1) {
    printf("failed to read table count from catalog\n");
    fclose(fp);
    return;
  }

  table_count += 1;

  // Move the file pointer back to the beginning of the file
  if (fseek(fp, 0, SEEK_SET) != 0) {
    printf("failed to seek to beginning of file\n");
    fclose(fp);
    return;
  }

  if (fwrite(&table_count, sizeof(int), 1, fp) != 1) {
    printf("failed to increment table count\n");
  }
  fclose(fp);
}

Schema *read_catalog(char *db_loc) {
  char filepath[100];
  snprintf(filepath, sizeof(filepath), "%s/%s", db_loc, "catalog");
  FILE *fp = fopen(filepath, "rb");
  Schema *db_schemas = malloc(sizeof(Schema));

  if (fp == NULL) {
    printf("Failed to open file for reading\n");
    return NULL;
  }

  if (fseek(fp, 0, SEEK_SET) != 0) {
    printf("failed to seek to beginning of file\n");
    fclose(fp);
    return NULL;
  }

  int table_count;
  // read #table count
  if (fread(&table_count, sizeof(int), 1, fp) != 1) {
    printf("failed to read table count from catalog\n");
    fclose(fp);
    return NULL;
  }

  db_schemas->num_tables = table_count;
  db_schemas->tables = malloc(sizeof(Table) * 100);

  // printf("num tables stored in catalog: %d\n", table_count);

  for (int i = 0; i < table_count; i++) {
    int table_name_len;
    if (fread(&table_name_len, sizeof(int), 1, fp) != 1) {
      printf("failed to read %d table name len from catalog\n", i);
      fclose(fp);
      return NULL;
    }

    char *table_name = malloc(sizeof(char) * table_name_len + 1);
    if (fread(table_name, sizeof(char), table_name_len, fp) != table_name_len) {
      printf("failed to read first table name from catalog\n");
      fclose(fp);
      return NULL;
    }
    table_name[table_name_len] = '\0';
    // printf("CREATED TABLE NAME: %s %d\n", table_name, table_name_len);

    db_schemas->tables[i].name = table_name;

    // printf("attempting to read table #%d name: %s...\n", i, table_name);

    int num_attributes;
    if (fread(&num_attributes, sizeof(int), 1, fp) != 1) {
      printf("failed to read num_attributes from table #%d\n", i);
      fclose(fp);
      return NULL;
    }

    // printf("table #%d num_attribute: %d\n", i, num_attributes);

    db_schemas->tables[i].num_attributes = num_attributes;
    db_schemas->tables[i].attributes =
        malloc(sizeof(Attribute) * num_attributes);
    // loop over attributes
    for (int j = 0; j < num_attributes; j++) {

      Attribute *attribute_ptr = malloc(sizeof(Attribute));

      // attribute name length
      int attr_name_len;
      if (fread(&attr_name_len, sizeof(int), 1, fp) != 1) {
        printf("failed to read attr #%d attr_name_len from table #%d\n", j, i);
        fclose(fp);
        return NULL;
      }

      // actual attribute name
      char *attr_name = malloc(sizeof(char) * attr_name_len + 1);
      if (fread(attr_name, sizeof(char), attr_name_len, fp) != attr_name_len) {
        printf("failed to read attr #%d attr name from table #%d\n", j, i);
        fclose(fp);
        return NULL;
      }
      attr_name[attr_name_len] = '\0';
      attribute_ptr->name = attr_name;

      // determine what type (integer, bool etc)
      int attr_type;
      if (fread(&attr_type, sizeof(int), 1, fp) != 1) {
        printf("failed to read attr #%d attr_type from table #%d\n", j, i);
        fclose(fp);
        return NULL;
      }

      attribute_ptr->type = int_to_attribute_type(attr_type);

      // need to read len for char/varchar
      if (attr_type == 3 || attr_type == 4) {
        int attr_len;
        if (fread(&attr_len, sizeof(int), 1, fp) != 1) {
          printf("failed to read attr #%d attr_len from table #%d\n", j, i);
          fclose(fp);
          return NULL;
        }
        attribute_ptr->len = attr_len;
      }

      int is_primary_key;
      if (fread(&is_primary_key, sizeof(int), 1, fp) != 1) {
        printf("failed to read attr #%d primary_key ness from table #%d\n", j,
               i);
        fclose(fp);
        return NULL;
      }
      if (is_primary_key == 1) {
        attribute_ptr->is_primary_key = true;
      } else {
        attribute_ptr->is_primary_key = false;
      }

      int is_notnull;
      if (fread(&is_notnull, sizeof(int), 1, fp) != 1) {
        printf("failed to read attr #%d notnull ness from table #%d\n", j, i);
        fclose(fp);
        return NULL;
      }
      if (is_notnull == 1) {
        attribute_ptr->notnull = true;
      } else {
        attribute_ptr->notnull = false;
      }

      int is_unique;
      if (fread(&is_unique, sizeof(int), 1, fp) != 1) {
        printf("failed to read attr #%d unique ness from table #%d\n", j, i);
        fclose(fp);
        return NULL;
      }
      if (is_unique == 1) {
        attribute_ptr->unique = true;
      } else {
        attribute_ptr->unique = false;
      }

      // printf("attr #%d name: %s , type: %s , is_primary_key: %d\n", j,
      //        attr_name, attribute_type_to_string(attr_type), is_primary_key);
      db_schemas->tables[i].attributes[j] = *attribute_ptr;
    }
  }
  fclose(fp);
  return db_schemas;
}

// add a singular table to the catalog
// starts by incrementing the first byte stored in the catalog file
void write_catalog(char *db_loc, Table *table) {

  increment_table_count(db_loc);

  char filepath[100];
  snprintf(filepath, sizeof(filepath), "%s/%s", db_loc, "catalog");

  // needs to be ab+ so that data isnt erased when opening file
  FILE *fp = fopen(filepath, "ab+");
  // Move the file pointer to the end of the file
  if (fseek(fp, 0, SEEK_END) != 0) {
    printf("Failed to seek to end of file\n");
    fclose(fp);
    return;
  }

  int table_name_len = strlen(table->name);
  // printf("table name len: %d\n", table_name_len);

  if (fwrite(&table_name_len, sizeof(int), 1, fp) != 1) {
    printf("failed to write table_name_len\n");
    fclose(fp);
    return;
  }

  // write table name
  if (fwrite(table->name, sizeof(char), table_name_len, fp) != table_name_len) {
    printf("failed to write table_name\n");
    fclose(fp);
    return;
  }

  // write #attributes
  if (fwrite(&table->num_attributes, sizeof(int), 1, fp) != 1) {
    printf("failed to write num_attributes\n");
    fclose(fp);
    return;
  }

  for (int i = 0; i < table->num_attributes; i++) {
    Attribute *curr_attribute = &table->attributes[i];
    int attr_name_len = strlen(curr_attribute->name);
    // printf("attr name: %s \n", curr_attribute->name);
    // printf("attr len: %d \n", attr_name_len);
    if (fwrite(&attr_name_len, sizeof(int), 1, fp) != 1) {
      printf("failed to write attr_name_len\n");
      fclose(fp);
      return;
    }
    if (fwrite(curr_attribute->name, sizeof(char), attr_name_len, fp) !=
        attr_name_len) {
      printf("failed to write attr_name\n");
      fclose(fp);
      return;
    }

    int attr_type = attribute_type_to_int(curr_attribute->type);
    // printf("attr_type is: %d\n", attr_type);

    if (fwrite(&attr_type, sizeof(int), 1, fp) != 1) {
      printf("failed to write attr_type\n");
      fclose(fp);
      return;
    }

    // char/varchar need to write a byte for len
    if (attr_type == 3 || attr_type == 4) {
      if (fwrite(&curr_attribute->len, sizeof(int), 1, fp) != 1) {
        printf("failed to write attr_len\n");
        fclose(fp);
        return;
      }
    }

    // write if part of primary key (0 for false, 1 for true)
    int is_primary_key = curr_attribute->is_primary_key ? 1 : 0;
    if (fwrite(&is_primary_key, sizeof(int), 1, fp) != 1) {
      printf("failed to write attr primary_key ness\n");
      fclose(fp);
      return;
    }
  }

  fclose(fp);
}

void add_table_to_catalog(Schema *db_schema, Table *table) {
  db_schema->num_tables += 1;
  db_schema->tables =
      realloc(db_schema->tables, sizeof(Table) * db_schema->num_tables);
  db_schema->tables[db_schema->num_tables - 1] = *table;
  db_schema->btrees =
      realloc(db_schema->btrees, sizeof(BPlusTree) * db_schema->num_tables);
  Attribute attr;
  for (int j = 0; j < table->num_attributes; j++) {
    if (table->attributes[j].is_primary_key) {
      attr = table->attributes[j];
    }
  }
  printf("Creating index for attribute '%s' in table '%s'\n", attr.name,
         table->name);
  int max_size = 4;
  if (attr.type == INTEGER) {
    max_size += 4;
  } else if (attr.type == DOUBLE) {
    max_size += 8;
  } else if (attr.type == BOOL) {
    max_size += 4;
  } else if (attr.type == CHAR) {
    max_size += attr.len;
  } else if (attr.type == VARCHAR) {
    max_size += attr.len;
  }
  int b_tree_n = db_schema->page_size / max_size - 1;
  BPlusTree *index = init_BPlusTree(b_tree_n, true, false);
  db_schema->btrees[db_schema->num_tables - 1] = *index;
}

void create_catalog(char *db_loc) {
  char filepath[100];
  snprintf(filepath, sizeof(filepath), "%s/%s", db_loc, "catalog");
  // printf("%s is filepath\n", filepath);
  int table_count = 0;
  FILE *fp = fopen(filepath, "wb");
  if (fwrite(&table_count, sizeof(int), 1, fp) != 1) {
    printf("Failed to initialize table count\n");
    printf("ERROR\n");
  }
  fclose(fp);
  // printf("done creating catalog\n");
}

Table *get_table(Schema *db_schema, char *table_name) {
  for (int i = 0; i < db_schema->num_tables; i++) {
    Table *t = &db_schema->tables[i];
    if (strcmp(t->name, table_name) == 0) {
      return t;
    }
  }
  return NULL;
}

bool drop_table(Schema *db_schema, struct bufferm *buffer, char *table_name) {
  Table *old_table = get_table(db_schema, table_name);
  if (old_table == NULL) {
    printf("%s does not exist\n", table_name);
    return false;
  } else {
    // printf("dropping the %s table\n", table_name);
  }

  // remove from buffer
  Page *old_page = remove_from_buffer(buffer, old_table);

  char filepath[100];
  //  remove table file
  snprintf(filepath, sizeof(filepath), "%s/%s", db_schema->db_path, table_name);

  if (remove(filepath) == 0) {
    // printf("%s was removed\n", filepath);
  } else {
    printf("%s was not removed for some reason\n", filepath);
  }

  // update schema
  int table_index = 0;
  for (int i = 0; i < db_schema->num_tables; i++) {
    if (strcmp(db_schema->tables[i].name, table_name) == 0) {
      table_index = i;
      break;
    }
  }

  // move last table to index of table that is going to be removed
  db_schema->tables[table_index] = db_schema->tables[db_schema->num_tables - 1];
  db_schema->num_tables -= 1;
  // db_schema->tables =
  //  realloc(db_schema->tables, sizeof(Table) * db_schema->num_tables);

  return true;
}

bool alter_table_add(Schema *db_schema, struct bufferm *buffer,
                     char *table_name, Attribute *attr,
                     Attribute_Values *attr_val) {

  // printf("in alter table add\n");
  char filepath[100];
  snprintf(filepath, sizeof(filepath), "%s/%s", db_schema->db_path, table_name);

  Table *old_table = get_table(db_schema, table_name);
  if (old_table == NULL) {
    printf("%s does not exist\n", table_name);
    return false;
  } else {
    // printf("updating the %s table\n", table_name);
  }

  for (int i = 0; i < old_table->num_attributes; i++) {
    if (strcmp(old_table->attributes[i].name, attr->name) == 0) {
      printf("attr name %s already exists for table %s\n", attr->name,
             table_name);
      return false;
    }
  }

  Table *new_table = malloc(sizeof(Table));
  new_table->name = malloc(strlen(table_name) + 1);
  strcpy(new_table->name, table_name);
  new_table->num_attributes = old_table->num_attributes + 1;
  new_table->attributes = malloc(sizeof(Attribute) * new_table->num_attributes);

  // copy old attributes
  for (int i = 0; i < old_table->num_attributes; i++) {
    new_table->attributes[i] = old_table->attributes[i];
  }

  // insert new attr
  new_table->attributes[new_table->num_attributes - 1] = *attr;

  // printf("printing new table metadata\n");
  // print_table_metadata(new_table);

  Page *p = find_in_buffer(buffer, old_table);
  if (p == NULL) {
    // printf("%s page was not in the buffer\n", table_name);
    p = read_page_from_file(db_schema, old_table, filepath);
  }
  if (p == NULL) {
    // printf("%s page was null, going to update schema tho!\n", table_name);
    // no pages for this table
    for (int i = 0; i < db_schema->num_tables; i++) {
      if (strcmp(db_schema->tables[i].name, table_name) == 0) {
        db_schema->tables[i] = *new_table;
      }
    }
    // rewrite entire schemas to catalog file
    write_schemas_to_catalog(db_schema);
    return true;
  }

  // printf("old # attr: %d new # attr:%d\n", old_table->num_attributes,
  //        new_table->num_attributes);

  // if (attr_val != NULL) {
  //   printf("%d is default int_val to be inserted\n", attr_val->int_val);
  // } else {
  //   printf("should insert nulls to records\n");
  // }

  // remove old table from buffer
  Page *old_page = remove_from_buffer(buffer, old_table);

  //  remove table file
  snprintf(filepath, sizeof(filepath), "%s/%s", db_schema->db_path, table_name);

  if (remove(filepath) == 0) {
    // printf("%s was removed\n", filepath);
  } else {
    // printf("%s was not removed for some reason\n", filepath);
  }

  // I NEED TO UPDATE BITMAP
  // AS WELL AS RECORD SIZE
  // ALSO COPY PRIMARY KEY INDEX
  // NEED TO CLONE ATTR_VALS
  // NEED TO REMOVE FROM BUFFER AS WELL
  int page_count = 0;
  // loop through all pages
  while (p != NULL) {
    // printf("%d num records to update on page: %d\n", p->num_records,
    //        ++page_count);
    for (int i = 0; i < p->num_records; i++) {
      Record *record = &p->records[i];
      // make space for new column
      // record->attr_vals =
      //     realloc(record->attr_vals,
      //             sizeof(Attribute_Values) * new_table->num_attributes);

      Record *new_record = malloc(sizeof(Record));
      new_record->attr_vals =
          malloc(sizeof(Attribute_Values) * new_table->num_attributes);
      new_record->primary_key_index = record->primary_key_index;
      new_record->bitmap = record->bitmap;

      Attribute_Values *new_attr_val;
      if (attr_val == NULL) {
        new_attr_val = malloc(sizeof(Attribute_Values));
        new_attr_val->type = attr->type;
        new_attr_val->is_null = true;
      } else {
        // clone
        new_attr_val = clone_attr_vals(attr_val);
      }
      // printf("new attr type: %s\n",
      //        attribute_type_to_string(new_attr_val->type));

      // printf("old record size: %d\n", calculate_record_size(old_table,
      // record)); add new attr_val to record
      for (int old_i = 0; old_i < old_table->num_attributes; old_i++) {
        new_record->attr_vals[old_i] = record->attr_vals[old_i];
      }

      new_record->attr_vals[new_table->num_attributes - 1] = *new_attr_val;
      // printf("new record size: %d\n", calculate_record_size(new_table,
      // record));

      // set new record size
      new_record->size = calculate_record_size(new_table, new_record);

      // printf("new record size: %d, page max size: %d\n", new_record->size,
      //        db_schema->page_size);

      // printf("type: %s int_val: %d\n",
      //        attribute_type_to_string(new_attr_val->type),
      //        attr_val->int_val);

      // UPDATE BIT MAP
      if (attr_val != NULL) {
        new_record->bitmap |= (1 << old_table->num_attributes);
      }
      // for (int i = 0; i < new_table->num_attributes; i++) {
      //   if ((new_record->bitmap & (1 << i)) != 0) {
      //     printf("1");
      //   } else {
      //     printf("0");
      //   }
      // }
      // printf("\n");

      Page *new_page =
          add_record_to_page(db_schema, new_table, new_record, buffer);
      if (new_page == NULL) {
        printf("failed to insert new attr %s\n", attr->name);
        if (old_page != NULL) {
          char filepath[100];
          snprintf(filepath, sizeof(filepath), "%s/%s", db_schema->db_path,
                   old_table->name);
          add_to_buffer(buffer, old_table, old_page, filepath);
        }
        return false;
      }
    }
    if (p->next_page != NULL) {
      p = p->next_page;
    } else {
      break;
    }
  }

  for (int i = 0; i < db_schema->num_tables; i++) {
    if (strcmp(db_schema->tables[i].name, table_name) == 0) {
      db_schema->tables[i] = *new_table;
    }
  }
  write_schemas_to_catalog(db_schema);
  return true;
}

bool alter_table_drop(Schema *db_schema, struct bufferm *buffer,
                      char *table_name, char *attr_name) {

  // printf("in alter table drop %s for table %s\n", attr_name, table_name);
  char filepath[100];
  snprintf(filepath, sizeof(filepath), "%s/%s", db_schema->db_path, table_name);

  Table *old_table = get_table(db_schema, table_name);
  if (old_table == NULL) {
    printf("%s does not exist\n", table_name);
    return false;
  } else {
    // printf("updating the %s table to drop %s\n", table_name, attr_name);
  }

  bool found_attr = false;
  int index_of_attr_to_drop = 0;
  // check if attr_name actually exists in table and track index
  for (int i = 0; i < old_table->num_attributes; i++) {
    if (strcmp(old_table->attributes[i].name, attr_name) == 0) {
      // check if attempting to drop primary key
      if (old_table->attributes[i].is_primary_key == true) {
        printf("Cannot drop primary key %s from %s\n", attr_name, table_name);
        return false;
      }
      found_attr = true;
      index_of_attr_to_drop = i;
      break;
    }
  }

  if (!found_attr) {
    printf("attr: %s is not in table: %s\n", attr_name, table_name);
    return false;
  } else {
    // printf("attr: %s was in table, going to drop it\n", attr_name);
  }

  Table *new_table = malloc(sizeof(Table));
  new_table->name = malloc(strlen(table_name) + 1);
  strcpy(new_table->name, table_name);
  new_table->num_attributes = old_table->num_attributes - 1;
  new_table->attributes = malloc(sizeof(Attribute) * new_table->num_attributes);

  // copy old attributes except the one to drop
  int j = 0;
  for (int i = 0; i < old_table->num_attributes; i++) {
    if (i == index_of_attr_to_drop) {
      continue;
    }
    new_table->attributes[j] = old_table->attributes[i];
    j += 1;
  }
  Page *p = find_in_buffer(buffer, old_table);
  if (p == NULL) {
    // printf("%s page was not in the buffer\n", table_name);
    p = read_page_from_file(db_schema, old_table, filepath);
  }
  if (p == NULL) {
    // printf("%s page was null, going to update schema to drop tho!\n",
    //        table_name);
    // no pages for this table
    for (int i = 0; i < db_schema->num_tables; i++) {
      if (strcmp(db_schema->tables[i].name, table_name) == 0) {
        db_schema->tables[i] = *new_table;
      }
    }
    // rewrite entire schemas to catalog file
    // write_schemas_to_catalog(db_schema);
    return true;
  }

  // remove old table from buffer
  remove_from_buffer(buffer, old_table);

  //  remove table file
  snprintf(filepath, sizeof(filepath), "%s/%s", db_schema->db_path, table_name);

  if (remove(filepath) == 0) {
    // printf("%s was removed\n", filepath);
  } else {
    // printf("%s was not removed for some reason\n", filepath);
  }

  // I NEED TO UPDATE BITMAP
  // AS WELL AS RECORD SIZE
  // ALSO COPY PRIMARY KEY INDEX
  // NEED TO CLONE ATTR_VALS
  // NEED TO REMOVE FROM BUFFER AS WELL
  int page_count = 0;
  // loop through all pages
  while (p != NULL) {
    for (int i = 0; i < p->num_records; i++) {
      Record *record = &p->records[i];
      Record *new_record = malloc(sizeof(Record));
      new_record->attr_vals =
          malloc(sizeof(Attribute_Values) * new_table->num_attributes);
      int pos = 0;

      for (int j = 0; j < old_table->num_attributes; j++) {
        if (j == index_of_attr_to_drop) {
          continue;
        }
        new_record->attr_vals[pos] = record->attr_vals[j];
        pos += 1;
      }
      new_record->size = calculate_record_size(new_table, new_record);

      // printf("old record size:%d new record size:%d\n", record->size,
      //        new_record->size);

      int new_bitmap = 0;
      for (int attr_index = 0; attr_index < new_table->num_attributes;
           attr_index++) {
        if (new_record->attr_vals[attr_index].is_null) {
          continue;
        }
        new_bitmap |= (1 << attr_index);
      }

      new_record->bitmap = new_bitmap;

      // for (int i = 0; i < new_table->num_attributes; i++) {
      //   if ((new_record->bitmap & (1 << i)) != 0) {
      //     printf("1");
      //   } else {
      //     printf("0");
      //   }
      // }
      // printf("\n");

      // UPDATE BIT MAP
      Page *new_page = add_record_to_page(db_schema, new_table, record, buffer);
      if (new_page == NULL) {
        return false;
      }
    }
    if (p->next_page != NULL) {
      p = p->next_page;
    } else {
      break;
    }
  }

  for (int i = 0; i < db_schema->num_tables; i++) {
    if (strcmp(db_schema->tables[i].name, table_name) == 0) {
      db_schema->tables[i] = *new_table;
    }
  }
  return true;
}

void write_schemas_to_catalog(Schema *db_schema) {

  // clear contents of catalog file
  char filepath[100];
  snprintf(filepath, sizeof(filepath), "%s/%s", db_schema->db_path, "catalog");

  FILE *fp = fopen(filepath, "wb+");
  if (fwrite(&db_schema->num_tables, sizeof(int), 1, fp) != 1) {
    printf("failed to increment table count\n");
  }

  for (int t = 0; t < db_schema->num_tables; t++) {
    Table *table = &db_schema->tables[t];
    int table_name_len = strlen(table->name);
    // printf("table name len: %d\n", table_name_len);

    if (fwrite(&table_name_len, sizeof(int), 1, fp) != 1) {
      printf("failed to write table_name_len\n");
      fclose(fp);
      return;
    }

    // write table name
    if (fwrite(table->name, sizeof(char), table_name_len, fp) !=
        table_name_len) {
      printf("failed to write table_name\n");
      fclose(fp);
      return;
    }

    // write #attributes
    if (fwrite(&table->num_attributes, sizeof(int), 1, fp) != 1) {
      printf("failed to write num_attributes\n");
      fclose(fp);
      return;
    }

    for (int i = 0; i < table->num_attributes; i++) {
      Attribute *curr_attribute = &table->attributes[i];
      int attr_name_len = strlen(curr_attribute->name);
      // printf("attr name: %s \n", curr_attribute->name);
      // printf("attr len: %d \n", attr_name_len);
      if (fwrite(&attr_name_len, sizeof(int), 1, fp) != 1) {
        printf("failed to write attr_name_len\n");
        fclose(fp);
        return;
      }
      if (fwrite(curr_attribute->name, sizeof(char), attr_name_len, fp) !=
          attr_name_len) {
        printf("failed to write attr_name\n");
        fclose(fp);
        return;
      }

      int attr_type = attribute_type_to_int(curr_attribute->type);
      // printf("attr_type is: %d\n", attr_type);

      if (fwrite(&attr_type, sizeof(int), 1, fp) != 1) {
        printf("failed to write attr_type\n");
        fclose(fp);
        return;
      }

      // char/varchar need to write a byte for len
      if (attr_type == 3 || attr_type == 4) {
        if (fwrite(&curr_attribute->len, sizeof(int), 1, fp) != 1) {
          printf("failed to write attr_len\n");
          fclose(fp);
          return;
        }
      }

      // write if part of primary key (0 for false, 1 for true)
      int is_primary_key = curr_attribute->is_primary_key ? 1 : 0;
      if (fwrite(&is_primary_key, sizeof(int), 1, fp) != 1) {
        printf("failed to write attr primary_key ness\n");
        fclose(fp);
        return;
      }

      // write if notnull constraint
      int notnull = curr_attribute->notnull ? 1 : 0;
      if (fwrite(&notnull, sizeof(int), 1, fp) != 1) {
        printf("failed to write attr notnull ness\n");
        fclose(fp);
        return;
      }

      // write if unique constraint
      int unique = curr_attribute->unique ? 1 : 0;
      if (fwrite(&unique, sizeof(int), 1, fp) != 1) {
        printf("failed to write attr unique ness\n");
        fclose(fp);
        return;
      }
    }
  }

  fclose(fp);
}

Attribute_Values *clone_attr_vals(Attribute_Values *src) {
  if (src == NULL) {
    // printf("src was null...");
    return NULL;
  }
  // printf("about to clone attr val\n");
  Attribute_Values *attr_val = malloc(sizeof(Attribute_Values));
  attr_val->type = src->type;

  ATTRIBUTE_TYPE type = attr_val->type;
  // printf("set type %s\n", attribute_type_to_string(attr_val->type));
  if (type == INTEGER) {
    attr_val->int_val = src->int_val;
  } else if (type == DOUBLE) {
    attr_val->double_val = src->double_val;
  } else if (type == BOOL) {
    attr_val->bool_val = src->bool_val;
  } else if (type == CHAR) {
    attr_val->chars_val = malloc(sizeof(char) * strlen(src->chars_val) + 1);
    strcpy(attr_val->chars_val, src->chars_val);
  } else if (type == VARCHAR) {
    attr_val->chars_val = malloc(sizeof(char) * strlen(src->chars_val) + 1);
    strcpy(attr_val->chars_val, src->chars_val);
  }

  attr_val->is_null = src->is_null;
  return attr_val;
}
