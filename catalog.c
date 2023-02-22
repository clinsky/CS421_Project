#include "table.h"
#include "catalog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
// Schema
struct schema {
    char db_path[100];
    unsigned int page_size;
    unsigned int buffer_size;
    unsigned int num_tables;
};

// Each attribute contains a type and a name
struct attribute {
    char attribute_name[50];
    char attribute_type[50];
};

// Each table contains an arbitrary number of attributes, a number of pages, and a number of records.
struct table {
    char table_name[50];
    struct attribute table_attributes[50];
    unsigned int num_records;
    unsigned int num_pages;
    unsigned int * page_locations;
};
 */

int write_catalog(struct schema db_schema, struct table* tables)
{
    FILE* fp = fopen("catalog", "wb");
    fwrite(&db_schema, sizeof(struct schema), 1, fp);
    for(int i = 0; i < db_schema.num_tables; i++)
    {
        printf("%i", i);
        fwrite(&tables[i], sizeof(struct table), 1, fp);
    }
    return 1;
}

int read_catalog()
{
    FILE* fp = fopen("catalog", "rb");
    struct schema db_schema;
    fread(&db_schema, sizeof(struct schema), 1, fp);
    printf(db_schema.db_path);
    printf("\n");
    printf("%i\n", db_schema.num_tables);
    
    struct table bar;
    fread(&bar, sizeof(struct table), 1, fp);
    printf(bar.name);
    printf("\n");
    printf("%i\n", bar.num_attributes);
}

Schema create_schema(char * db_loc, int page_size, int buffer_size){
    Schema db_schema;
    strncpy(db_schema.db_path, "some_path/db", sizeof(db_schema.db_path));
    db_schema.page_size = page_size;
    db_schema.buffer_size = buffer_size;
    db_schema.num_tables = 0;
    return db_schema;
}


int create_catalog()
{
    // Create schema
    struct schema db_schema;
    strncpy(db_schema.db_path, "some_path/db", sizeof(db_schema.db_path));
    db_schema.page_size = 4095;
    db_schema.buffer_size = 25;
    db_schema.num_tables = 0;
    
    // Creating table
    struct attribute x;
    strncpy(x.name, "x", sizeof(x.name));
    strncpy(x.type, "double", sizeof(x.type));
    
    struct attribute y;
    strncpy(y.name, "y", sizeof(y.name));
    strncpy(y.type, "char(5)", sizeof(y.type));
    
    struct table bar;
    bar.attributes[0] = x;
    bar.attributes[1] = y;
    strncpy(bar.name, "bar", strlen(bar.name));
    bar.num_attributes = 1;
    
    // Update schema to have 1 table.
    db_schema.num_tables = 1;
    
    // Write catalog
    struct table tables[1];
    tables[0] = bar;
    write_catalog(db_schema, tables);
    
    // Read catalog and print examples
    read_catalog();
}