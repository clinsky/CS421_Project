#include "buffer.h"

Bufferm *create_new_bufferm(int max_pages) {
  Bufferm *b = malloc(sizeof(Bufferm));
  b->max_pages = max_pages;
  b->pages = malloc(sizeof(Page) * max_pages + 2);
  return b;
}
