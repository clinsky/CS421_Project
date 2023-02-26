#include "catalog.h"

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

      // printf("attr #%d name: %s , type: %s , is_primary_key: %d\n", j,
      //        attr_name, attribute_type_to_string(attr_type), is_primary_key);
      db_schemas->tables[i].attributes[j] = *attribute_ptr;
    }
  }
  fclose(fp);
  return db_schemas;
}

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
