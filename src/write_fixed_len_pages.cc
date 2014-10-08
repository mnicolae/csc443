#include <stdio.h>
#include <fstream>
#include <string.h>
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

int main(int argc, char *argv[])
{
  int rc = 0;
  char * delimeter = ",";
  char * attr;
  int ch;
  int count = 0;
  Record record;
  Page page;
  int numRecords = 0;
  int numPages = 1;
  int page_size;
  int slot_size; 
  char line[1200];
  struct timeb start, end;
  ifstream csvStream;
  FILE * csvNumLines;
  FILE * pageStream;

  if (argc != 4)
  {
    printf("Usage: write_fixed_len_pages <csv_file> <page_file> <page_size>\n");
  }
  
  csvStream.open(argv[1]);
  csvNumLines = fopen(argv[1], "r");
  pageStream = fopen(argv[2], "w");

  page_size = atoi(argv[3]);
  slot_size = calculate_slot_size(page_size, RECORD_SIZE);
  assert(page_size > RECORD_SIZE + PAGE_STRUCT_SIZE);

  init_fixed_len_page(&page, page_size, slot_size); 
  
  do
  {
      ch = fgetc(csvNumLines);
      if (ch == '\n') count++;
  } while (ch != EOF);
 
  ftime(&start);
  long start_time = start.time * 1000 + start.millitm; 

  for (int i = 0; i < count; i++)
  {
     attr = NULL;
     csvStream.getline(line, 1200);
     attr = strtok(line, delimeter);

     // get the record   
     while (attr)
     {
       record.push_back(attr);
       attr = strtok(NULL, delimeter);
     }
     
     // write the record to the page
     rc = add_fixed_len_page(&page, &record); 
     numRecords++;
     if (rc == -1)
     {
        // if the current page is full, write it to page file, allocate a new page
	fwrite(page.data, 1, page.page_size, pageStream);
        init_fixed_len_page(&page, page_size, slot_size); 
        rc = add_fixed_len_page(&page, &record); 
        numPages++;
     }
  }

  // write the last page to pagefile
  fwrite(page.data, 1, page.page_size, pageStream);

  ftime(&end);
  long end_time = end.time * 1000 + end.millitm; 

  printf("\nNUMBER OF RECORDS: %d", numRecords);
  printf("\nNUMBER OF PAGES: %d", numPages);
  printf("\nTIME: %ld miliseconds\n", end_time - start_time);
  csvStream.close();
  fclose(pageStream);
  fclose(csvNumLines);

  return 0;
}
