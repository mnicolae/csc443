#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <vector>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <sys/timeb.h>
#include <ctime>
#include "heapManager.h"

/* Select single attribute with parameters <start> and <end>
 * <attribute_id> is the index of the attribute to be selected 
 * (e.g. 0 for the first attribute, 1 for the second attribute, etc.)
 */
int main(int argc, char *argv[])
{
   Page dirPage;
   Page dataPage;
   FILE * heapFile;
   Heapfile heapfile;
   int page_size;
   int slot_size; 
   DirectoryEntry dirEntry, nextDirPointer; 
   int capacity, attrNo;
   int dirOffset = 0;
   const char * start, * end;
   char * empty = "          ";
   struct timeb startTime, endTime;

   if (argc != 6)
   {
     printf("Usage: select2 <colstore_name> <attribute_id> <start> <end> <page_size>\n");
   }

   string name1 = argv[1];
   string name2 = argv[2];

   string fileName = name1 + name2;
   const char *name = fileName.c_str();

   heapFile = fopen(name, "r");
   start = argv[3];
   end = argv[4];
   page_size = atoi(argv[5]);
   capacity = page_size / DIR_ENTRY_SIZE;
   slot_size = calculate_slot_size(page_size, 20);
   init_heapfile(&heapfile, page_size, heapFile);

   ftime(&startTime);
   long start_time = startTime.time * 1000 + startTime.millitm; 
 
   // read out first directory page and first directory page pointer
   fseek(heapfile.file_ptr, 0, SEEK_SET);
   init_fixed_len_page(&dataPage, page_size, slot_size);
   init_directory_page(&dirPage, page_size, 20); 

   fread(dirPage.data, page_size, 1, heapFile);
   
   memcpy((void *) &nextDirPointer, dirPage.data, DIR_ENTRY_SIZE); 

   do
   {
      for (int i = 1; i < capacity; i++)
      {
        // get each directory entry
        memcpy((void *) &dirEntry, dirPage.data + DIR_ENTRY_SIZE * i, DIR_ENTRY_SIZE);
        if (dirEntry.page_offset != 0)
        {
          read_page(&heapfile, dirEntry.pid, &dataPage, 20);
          for (int i = 0; i < slot_size; i++)
          {
             Record record;
             read_fixed_len_page(&dataPage, i, &record, 20); 
             if (strncmp(record.at(1), empty, 10))
             {
               // check if the attribute satisfies constraints
               if (strncmp(start, record.at(1), 10) <= 0 && strncmp(record.at(1), end, 10) <= 0)
               {
                   printf("%s\n", record.at(1));
               }
             }
          }
        }
      }
   
     // will be used as the condition for do-while loop
     dirOffset = nextDirPointer.page_offset;

     // There exists another directory page
     if (nextDirPointer.page_offset != 0)
     {
        fseek(heapFile, nextDirPointer.page_offset, SEEK_SET);
        init_directory_page(&dirPage, page_size, 20); 
        fread(dirPage.data, page_size, 1, heapFile);
        memcpy((void *) &nextDirPointer, dirPage.data, DIR_ENTRY_SIZE);
     }
  } while (dirOffset != 0);

  ftime(&endTime);
  long end_time = endTime.time * 1000 + endTime.millitm; 
  printf("\nTIME: %ld miliseconds\n", end_time - start_time);

  fclose(heapFile);
  return 0;
}
