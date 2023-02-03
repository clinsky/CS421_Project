#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  char *db_loc = argv[1];
  int page_size = atoi(argv[2]);
  int buffer_size = atoi(argv[3]);

  printf("db_loc: %s\npage_size: %d\nbuffer_size:%d\n", db_loc, page_size,
         buffer_size);
}

struct page = {
	int * primary_keys; 
	int size; 
	int max_num_records;
	page * next;  
}; 

typedef struct page Page; 

Page * new_page(int max_num_records){ 
	return (Page *)malloc(max_num_records * 4 + 4 + 4 + 8); 
}

void * page_splitting(int * keys, int num_keys, int max_num_records){
	/* 
	Manage a database using the slotted page algorithm. 
		keys: array of primary keys to be inserted into the database 
		num_keys: the number of keys in the array of primary keys 
		max_num_records: the number of keys that can be in a page 
	*/ 
	
	// initialize a new page 
	Page * page_ptr = new_page(max_num_records);
	for (int count = 0; count < num_keys; count++){
		// try to insert into the page 
		if(page -> size >= max_num_records){ 
			page_ptr->next = split_page(page_ptr, *(keys+4*count));  
	
		}
		else{ 
			insert_key_into_page(page_ptr, *(keys+4*count)); 
		}

	} 
} 
