#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  char *db_loc = argv[1];
  int page_size = atoi(argv[2]);
  int buffer_size = atoi(argv[3]);

  printf("db_loc: %s\npage_size: %d\nbuffer_size:%d\n", db_loc, page_size,
         buffer_size);
}



