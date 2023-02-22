// Schema
struct schema {
    char db_path[100];
    unsigned int page_size;
    unsigned int buffer_size;
    unsigned int num_tables;
    unsigned int * page_locations;
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
};

typedef struct schema Schema;

Schema create_schema(char * db_loc, int page_size, int buffer_size);
