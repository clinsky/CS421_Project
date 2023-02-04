#include "page.h"

Page *new_page(int max_num_records) {

  Page *page_ptr = (Page *)malloc(max_num_records * 4 + 4 + 4 + 8);
  page_ptr->next = NULL;
  return page_ptr;
}

Page *page_splitting(int *keys, int num_keys, int max_num_records) {
  /*
  Manage a database using the slotted page algorithm.
          keys: array of primary keys to be inserted into the database
          num_keys: the number of keys in the array of primary keys
          max_num_records: the number of keys that can be in a page
  */

  // initialize a new page
  Page *page_ptr = new_page(max_num_records);
  for (int count = 0; count < num_keys; count++) {
    // try to insert into the page
    insert_key_into_page(page_ptr, (*keys + 4 * count));
  }
  return page_ptr;
}

void insert_key_into_page(Page *page_ptr, int pkey) {
  /*
          page_ptr: points to one of the pages that compose the database
          pkey: the primary key to be inserted into the database
  */

  // move through the pages to find where the pkey fits
  int count = 0;
  while (count < page_ptr->size) {
    if (pkey < *(4 * count + page_ptr->primary_keys)) {
      insert_key_into_page_location(page_ptr, pkey, count);
      break;
    }
    count++;
  }
  if (count == page_ptr->size) {
    insert_key_at_end_of_page(page_ptr, pkey);
  }
}

void insert_key_into_page_location(Page *page_ptr, int pkey, int count) {
  if (page_ptr->size == page_ptr->max_num_records) {
    split_page(page_ptr, pkey);
  } else {
    int temp = pkey;
    for (int idx = count; idx < page_ptr->size - 1; idx++) {
      *(page_ptr->primary_keys + 4 * (idx + 1)) =
          *(page_ptr->primary_keys + 4 * idx);
      *(page_ptr->primary_keys + 4 * idx) = temp;
      temp = *(page_ptr->primary_keys + 4 * (idx + 1));
    }
    (page_ptr->size)++;
  }
}

void insert_key_at_end_of_page(Page *page_ptr, int pkey) {
  if (page_ptr->size == page_ptr->max_num_records) {
    if (page_ptr->next == NULL) {
      split_page(page_ptr, pkey);
    } else {
      insert_key_into_page(page_ptr->next, pkey);
    }
  }

  else {
    *(page_ptr->primary_keys + 4 * (page_ptr->size - 1)) = pkey;
    (page_ptr->size)++;
  }
}

void split_page(Page *page_ptr, int pkey) {
  /*
          page_ptr: pointer to the page to be split
          pkey: a primary key to be inserted into the database
  */
  int first_page_size = (page_ptr->size) / 2;
  int second_page_size = (page_ptr->size) - first_page_size;
  Page *page_ptr2 = new_page(page_ptr->max_num_records);
  for (int idx = 0; idx < second_page_size; idx++) {
    *(page_ptr2->primary_keys + 4 * idx) =
        *(page_ptr->primary_keys + 4 * first_page_size + 4 * idx);
  }
  page_ptr->size = first_page_size;
  page_ptr2->size = second_page_size;
  if (page_ptr->next == NULL) {
    page_ptr->next = page_ptr2;
  } else {
    page_ptr2->next = page_ptr->next;
    page_ptr->next = page_ptr2;
  }
  insert_key_into_page(page_ptr, pkey);
}

void lmao() { printf("lmao"); }
