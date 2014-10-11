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
  FILE * heapFile, *heapFile1;
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
  
  // 10 bytes for the actual attributes + 10 bytes for the string representation
  // of the the tuple-id
  // NOTE: we don't really need 10 bytes to store the string representation of the
  // tuple-id, but it simplifies calculations.
  record_size = 10 + 10;
  slot_size = calculate_slot_size(page_size, record_size);
  diff = 0 - slot_size;
  assert(page_size > record_size + PAGE_STRUCT_SIZE);

  char recordArray[lineCount][100];
  Heapfile colstoreFile;

  // iterate over the csv file to parse the records into memory 
  for (int i = 0; i < lineCount; i++)
  {
     attr = NULL;
     csvStream.getline(line, 1200);
     attr = strtok(line, delimeter);
     
     int j = 0;
     while (attr)
     {
       strncat(recordArray[i], attr, 10);
       attr = strtok(NULL, delimeter);
       j++;
     }
  }

  // Now write the records in column store to disk
  for (int i = 0; i < 100; i++)
  {
      init_fixed_len_page(&page, page_size, slot_size); 

      // Null-terminate the strings just to be safe
      fileName[0] = '\0'; 
      fileNumber[0] = '\0';
  
      snprintf(fileNumber, sizeof(int), "%d", i);
  
      strncpy(fileName, colstore_name, strlen(colstore_name) + 1);
      //strncat(fileName, slash, strlen(slash));
      strncat(fileName, fileNumber, strlen(fileNumber));
  
      heapFile1 = fopen(fileName, "w");
      fclose(heapFile1);
      heapFile = fopen(fileName, "r+");
      init_heapfile(&colstoreFile, page_size, heapFile);

      for (int j = 0; j < lineCount; j++)
      {
         Record record;
         char attr[10];
         attr[0] = '\0';
         strncpy(attr, &recordArray[j][i], 10);

         // store the tuple id associated with the record
         snprintf(tuple_id, sizeof(int), "%d", j);
         record.push_back(tuple_id); 
         record.push_back(attr);

         rc = add_fixed_len_page(&page, &record, record_size);
         numRecords++;
         if (rc == -1)
         {
           pid = alloc_page(&colstoreFile);
           write_page(&page, &colstoreFile, pid);
           updateDirEntry(&colstoreFile, pid, diff);
           init_fixed_len_page(&page, page_size, slot_size); 
           rc = add_fixed_len_page(&page, &record, record_size); 
           numRecords = 0;
         }
      }

      numRecords += 1;
      pid = alloc_page(&colstoreFile);
      updateDirEntry(&colstoreFile, pid, 0 - numRecords);
      write_page(&page, &colstoreFile, pid);
    
      fclose(heapFile);
  }

  ftime(&end);
  long end_time = end.time * 1000 + end.millitm; 

  printf("\nTIME: %ld miliseconds\n", end_time - start_time);
  
  csvStream.close();
  fclose(heapFile);
  fclose(csvNumLines);
  
  return 0;
}
