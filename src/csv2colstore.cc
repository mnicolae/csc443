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
#include <string>
#include <stdlib.h>

using namespace std;

/* Build a column store from CSV file
 * <colstore_name> should be a file directory to store the heap files
 * $ csv2colstore <csv_file> <colstore_name> <page_size>
 */
int main(int argc, char *argv[])
{
  int rc = 0;
  char * delimeter = ",";
  const char * slash = "/";
  char fileNumber[sizeof(int) + 1];
  char * attr;
  int ch;
  int lineCount = 0;
  int numRecords = 0;
  Record record;
  Page page;
  int page_size;
  int slot_size, record_size; 
  char line[1200];
  ifstream csvStream;
  FILE * csvNumLines;
  FILE * heapFile;
  PageID pid;
  int diff;
  struct timeb start, end;
  char * colstore_name;
  char tuple_id[sizeof(int) + 1];
  char fileName[100];

  if (argc != 4)
  {
    printf("Usage: csv2colstore <csv_file> <colstore_name> <page_size>\n");
  }
  
  csvStream.open(argv[1]);
  csvNumLines = fopen(argv[1], "r");

  colstore_name = argv[2];
  page_size = atoi(argv[3]);
  
  // Create col store directory here

  ftime(&start);
  long start_time = start.time * 1000 + start.millitm; 
  
  // get the number of lines in the csv file 
  do
  {
     ch = fgetc(csvNumLines);
     if (ch == '\n') 
     {
       lineCount++;
     }
  } while (ch != EOF);
  
  record_size = 10 + sizeof(int); 
  slot_size = calculate_slot_size(page_size, record_size);
  diff = 0 - slot_size;
  assert(page_size > record_size + PAGE_STRUCT_SIZE);

  Record recordArray[lineCount];
  Heapfile colstoreFiles[lineCount];

  for (int w = 0; w < 100; w++)
  {
    snprintf(fileNumber, sizeof(int), "%d", w);
    strncpy(fileName, colstore_name, strlen(colstore_name) + 1);
    //strncat(fileName, slash, strlen(slash));
    //strncat(fileName, fileNumber, strlen(fileNumber));
    heapFile = fopen(fileName, "w");
    init_heapfile(&colstoreFiles[w], page_size, heapFile);
  }

  // iterate over the csv file to parse the records into memory 
  for (int i = 0; i < lineCount; i++)
  {
     attr = NULL;
     csvStream.getline(line, 1200);
     attr = strtok(line, delimeter);
   
     while (attr)
     {
       record.push_back(attr);
       attr = strtok(NULL, delimeter);
     }
     recordArray[i] = record;
  }

  // Now write the records in column store to disk
  for (int i = 0; i < 100; i++)
  {
      init_fixed_len_page(&page, page_size, slot_size); 

      for (int j = 0; j < lineCount; j++)
      {
         Record record;

         // store the tuple id associated with the record
         snprintf(tuple_id, sizeof(int), "%d", j);
         record.push_back(tuple_id); 
         record.push_back(recordArray[j].at(i));

         rc = add_fixed_len_page(&page, &record);
         numRecords++;
         if (rc == -1)
         {
           pid = alloc_page(&colstoreFiles[i]);
           write_page(&page, &colstoreFiles[i], pid);
           updateDirEntry(&colstoreFiles[i], pid, diff);
           init_fixed_len_page(&page, page_size, slot_size); 
           rc = add_fixed_len_page(&page, &record); 
           numRecords = 0;
         }
      }

      numRecords += 1;
      pid = alloc_page(&colstoreFiles[i]);
      updateDirEntry(&colstoreFiles[i], pid, 0 - numRecords);
      write_page(&page, &colstoreFiles[i], pid);
  }

  ftime(&end);
  long end_time = end.time * 1000 + end.millitm; 

  printf("\nTIME: %ld miliseconds\n", end_time - start_time);
  
  csvStream.close();
  fclose(heapFile);
  fclose(csvNumLines);
  
  return 0;
}
