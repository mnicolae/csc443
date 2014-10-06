#include <stdio.h>
#include <fstream>
#include <string.h>
#include "bufferManager.h"
#include "pageManager.h"
#include <vector>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <sstream>
#include <sys/timeb.h>
#include <assert.h>

using namespace std;

/**
* Read a record from the page from a given slot.
*/
int main(int argc, char *argv[])
{
  Page page;
  int page_size;
  int slot_size; 
  char line[1200];
  struct timeb start, end;

  FILE * pageStream;

  if (argc != 3)
  {
      printf("Usage: read_fixed_len_pages <page_file> <page_size>\n");
  }
  
  pageStream = fopen(argv[1], "r");

  page_size = atoi(argv[2]);
  slot_size = calculate_slot_size(page_size);
  assert(page_size > RECORD_SIZE + PAGE_STRUCT_SIZE);

  init_fixed_len_page(&page, page_size, slot_size); 
  
  ftime(&start);
  long start_time = start.time * 1000 + start.millitm; 
  
  fread(page.data, 1, page.page_size, pageStream);
  // read out all the pages and then read out all the records
  while(!feof(pageStream))
  {
     for(int i=0; i < slot_size; i++)
     {
        Record record;
	read_fixed_len_page(&page, i, &record);
        // print out a record
        for (int j=0; j < 100; j++)
        {
            if (j == 99)
            {
              printf("%s", record.at(j));
            }
            else
            {
	      printf("%s,", record.at(j));
            }
        }
        printf("\n");  
     }      
     fread(page.data, 1, page.page_size, pageStream);
  } 

  ftime(&end);
  long end_time = end.time * 1000 + end.millitm; 

  printf("\nTIME: %ld miliseconds\n", end_time - start_time);

  fclose(pageStream);

  return 0;
}
