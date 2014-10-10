#include <stdio.h>
#include <fstream>
#include <string.h>
#include <vector>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <assert.h>
#include "heapManager.h"

using namespace std;

// Insert all records in the CSV file to a heap file
int main(int argc, char *argv[])
{
  int slotNumber, page_size, slot_size, rc;
  Record record;
  Page page;
  char * attr;
  char line[1200];
  char * delimeter = ",";
  Heapfile hFile;
  FILE * heapFile;
  int ch;
  int count = 0;
  PageID pid;
  bool * slot;
  ifstream csvStream;
  FILE * csvNumLines;
  int availablePagePid;

  if (argc != 4)
  {
    printf("Usage: insert <heapfile> <csv_file> <page_size>\n");
  }

  heapFile = fopen(argv[1], "r+");
  csvStream.open(argv[2]);
  csvNumLines = fopen(argv[2], "r");
  page_size = atoi(argv[3]);

  init_heapfile(&hFile, page_size, heapFile);
  init_fixed_len_page(&page, page_size, slot_size); 

  slot_size = calculate_slot_size(page_size, RECORD_SIZE);
  assert(page_size > RECORD_SIZE + PAGE_STRUCT_SIZE);

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
     
     availablePagePid = findAvailablePage(&hFile);
     if (availablePagePid != -1)
     {
         read_page(&hFile, availablePagePid, &page);
         rc = add_fixed_len_page(&page, &record, RECORD_SIZE); 
         write_page(&page, &hFile, availablePagePid);
         updateDirEntry(&hFile, availablePagePid, -1);      
     }
     else
     {
        availablePagePid = alloc_page(&hFile);
        init_fixed_len_page(&page, page_size, slot_size);
        rc = add_fixed_len_page(&page, &record, RECORD_SIZE);
        write_page(&page, &hFile, availablePagePid);
        updateDirEntry(&hFile, availablePagePid, -1);
     }
  }

  fclose(heapFile);
  fclose(csvNumLines);
  csvStream.close();

  return 0;
}
