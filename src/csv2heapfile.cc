#include <stdio.h>
#include <fstream>
#include <string.h>
#include <vector>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <sstream>
#include <sys/timeb.h>
#include <assert.h>
#include "heapManager.h"

using namespace std;

int main(int argc, char *argv[])
{
  int rc = 0;
  char * delimeter = ",";
  char * attr;
  int ch;
  int count = 0;
  int numRecords = 0;
  Record record;
  Page page;
  int page_size;
  int slot_size; 
  char line[1200];
  ifstream csvStream;
  FILE * csvNumLines;
  Heapfile hFile;
  FILE * heapFile;
  PageID pid;
  int diff;
  struct timeb start, end;

  if (argc != 4)
  {
    printf("Usage: csv2heapfile <csv_file> <heapfile> <page_size>\n");
  }
  
  csvStream.open(argv[1]);
  csvNumLines = fopen(argv[1], "r");
  heapFile = fopen(argv[2], "r+");
  page_size = atoi(argv[3]);
  hFile.file_ptr = heapFile;
  hFile.page_size = page_size;

  slot_size = calculate_slot_size(page_size);
  diff = 0 - slot_size;
  assert(page_size > RECORD_SIZE + PAGE_STRUCT_SIZE);

  ftime(&start);
  long start_time = start.time * 1000 + start.millitm; 

  init_fixed_len_page(&page, page_size, slot_size); 
  
  // get the number of lines in the csv file 
  do
  {
      ch = fgetc(csvNumLines);
      if (ch == '\n') count++;
  } while (ch != EOF);
 
  for (int i = 0; i < count; i++)
  {
     attr = NULL;
     csvStream.getline(line, 1200);
     attr = strtok(line, delimeter);
   
     while (attr)
     {
       record.push_back(attr);
       attr = strtok(NULL, delimeter);
     }
     
     rc = add_fixed_len_page(&page, &record); 
     numRecords++;
     if (rc == -1)
     {
        pid = alloc_page(&hFile);
        write_page(&page, &hFile, pid);
        updateDirEntry(&hFile, pid, diff);
	//fwrite(page.data, 1, page.page_size, heapFile);
        init_fixed_len_page(&page, page_size, slot_size); 
        rc = add_fixed_len_page(&page, &record); 
        numRecords = 0;
     }
  }


  numRecords += 1;
  pid = alloc_page(&hFile);
  updateDirEntry(&hFile, pid, 0 - numRecords);
  write_page(&page, &hFile, pid);

  ftime(&end);
  long end_time = end.time * 1000 + end.millitm; 

  printf("\nTIME: %ld miliseconds\n", end_time - start_time);

  csvStream.close();
  fclose(heapFile);
  fclose(csvNumLines);

  

  return 0;

}

