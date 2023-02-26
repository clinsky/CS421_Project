#include "buffer.h"

Bufferm *create_new_bufferm(int max_pages) {
  Bufferm *b = malloc(sizeof(Bufferm));
  b->curr_pages = 0;
  b->max_pages = max_pages;
  b->entries = malloc(sizeof(Buffer_Entry) * max_pages);
  return b;
}

// void add_to_buffer(Bufferm *b, Page *p, char *table_name) {
//   Buffer_Entry new_entry = malloc(sizeof(Buffer_Entry));
// }
