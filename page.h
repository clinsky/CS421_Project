#ifndef PAGE_H
#define PAGE_H

#include "parse_utils.h"
#include "table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Page
struct page {
  char ***records;
  int num_records;
  int num_bytes;
  char *table_name;
  int page_index;
};

typedef struct page Page;

struct page_buffer{
  Page *pages;
  int last_used_count;
  int *last_used;
};

typedef struct page_buffer PageBuffer;

#endif
