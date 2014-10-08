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
#include <direct.h>
#include "heapManager.h"

using namespace std;

/* Build a column store from CSV file
 * <colstore_name> should be a file directory to store the heap files
 * $ csv2colstore <csv_file> <colstore_name> <page_size>
 */
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
  FILE * heapFile;
  PageID pid;
  int diff;
  struct timeb start, end;
  char * colstore_name;
  Record[] recordArray;
  Heapfile[] colstoreFiles;
  Heapfile heapF;
  char fileName;

  if (argc != 4)
  {
    printf("Usage: csv2colstore <csv_file> <colstore_name> <page_size>\n");
  }
  
  csvStream.open(argv[1]);
  csvNumLines = fopen(argv[1], "r");
  colstore_name = argv[2];
  page_size = atoi(argv[3]);

  mkdir(colstore_name);

  diff = 0 - slot_size;
  assert(page_size > RECORD_SIZE + PAGE_STRUCT_SIZE);

  ftime(&start);
  long start_time = start.time * 1000 + start.millitm; 
  
  // get the number of lines in the csv file 
  do
  {
     ch = fgetc(csvNumLines);
     if (ch == '\n') 
     {
       count++;
     }
  } while (ch != EOF);

  for (int w = 0; w < 100; w++)
  {
    strcpy(fileName, colstore_name);
    strcat(fileName, "/");
    strcat(fileName, itoa(count));
    heapFile = fopen(fileName, "r+");
    init_heapfile(&heapF, page_size, heapFile);
    colstoreFiles[count] = heapF;
  }

  slot_size = calculate_slot_size(page_size, 10 * count);

  init_fixed_len_page(&page, page_size, slot_size); 

  // iterate over the csv file to parse the records 
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

     RecordArray[i] = record;
  }

  for (int i = 0; i < 100; i++)
  {
      Record r;
      for (int j = 0; j < count; j++)
      {
         r.push_back(RecordArray[j].at(i));
      }
      rc = add_fixed_len_page(&page, &record);
      numRecords++;
      if (rc == -1)
      {
        pid = alloc_page(&hFile);
        write_page(&page, &colstoreFiles[i], pid);
        updateDirEntry(&hFile, pid, diff);
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
???
