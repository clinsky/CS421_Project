#ifndef BUFFER_H
#define BUFFER_H

#include "page.h"
#include "parse_utils.h"
#include "table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Schema
struct bufferm {
  int max_pages;
  int curr_pages;
  int counter;
  Page *pages;
};

typedef struct bufferm Bufferm;

Bufferm *create_new_bufferm(int max_pages);

Page *search_buffer(Bufferm *b);

void add_to_buffer(Bufferm *b, Page *p);

#endif
