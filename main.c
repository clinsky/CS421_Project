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
	Page * page_ptr =  (Page *)malloc(max_num_records * 4 + 4 + 4 + 8); 
	page_ptr -> next = Null; 
	return page_ptr; 
}

Page * page_splitting(int * keys, int num_keys, int max_num_records){
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
		insert_key_into_page(page_ptr, *(keys+4*count)); 
	}
	return page_ptr; 
} 

Void insert_key_into_page(Page * page_ptr, int pkey){ 
	/*
		page_ptr: points to one of the pages that compose the database 
		pkey: the primary key to be inserted into the database
	*/ 

	// move through the pages to find where the pkey fits 
	int count = 0; 
	while(count < page->size){
		if(pkey < *(4 * count + page->keys)){
			insert_key_into_page_location(page_ptr, pkey, count); 
			break; 
		}
		count++; 
	}	
	if(count == page->size){
		insert_key_at_end_of_page(page_ptr, pkey); 
	}
}

Void insert_key_into_page_location(Page * page_ptr, int pkey, int count){
	if(page_ptr -> size == page_ptr -> max_num_records){
		split_page(page_ptr, pkey); 
	}
	else{ 
		int temp = pkey; 
		for(int idx = count; idx < page->size - 1; idx++){
			*(page->keys + 4*(idx+1)) = *(page->keys + 4*idx); 
			*(page->keys + 4*idx) = temp; 
			temp = *(page->keys + 4*(idx+1));
		}
		(page->size)++; 
	} 
}  

Void insert_key_at_end_of_page(Page * page_ptr, int pkey){
	if(page_ptr -> size == page_ptr -> max_num_records){ 
		if(page_ptr -> next == Null){ 
			split_page(page_ptr, pkey); 
		}
		else{
			insert_key_into_page(page_ptr -> next, pkey); 
		}
	}
			
	else{
		*(page_ptr -> keys + 4*(page_ptr->size - 1)) = pkey; 
		(page_ptr -> size)++;  
	}

}

Void split_page(Page * page_ptr, int pkey){ 
	/* 
		page_ptr: pointer to the page to be split  
		pkey: a primary key to be inserted into the database 
	*/
	int first_page_size = (page->size) / 2; 
	int second_page_size = (page->size) - first_page_size; 
	Page * page_ptr2 = new_page(page_ptr->max_num_records); 
	for(int idx = 0; idx < second_page_size; idx++){
		*(page_ptr2 -> keys + 4*idx) = *(page_ptr->keys + 4*first_page_size + 4 * idx); 
	}	 
	page_ptr->size = first_page_size; 
	page_ptr2->size = second_page_size; 
	if(page_ptr->next == Null){ 
		page_ptr->next = page_ptr2; 
	} 
	else{
		page_ptr2->next = page_ptr->next; 
		page_ptr->next = page_ptr2;  
	} 
	insert_into_page(page_ptr, pkey); 
}

	
