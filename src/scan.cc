#include <stdio.h>
#include <fstream>
#include <string.h>
#include <vector>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include "heapManager.h"

// Print out all records in a heap file
int main(int argc, char *argv[])
{
   //Record record;
   Page dirPage;
   Page dataPage;
   FILE * heapFile;
   Heapfile heapfile;
   int page_size;
   int slot_size; 
   DirectoryEntry dirEntry, nextDirPointer; 
   int capacity;
   int dirOffset;
   char * empty = "          ";

   if (argc != 3)
   {
     printf("Usage: scan <heapfile> <page_size>\n");
   }

   page_size = atoi(argv[2]);
   slot_size = calculate_slot_size(page_size, 1000);
   capacity = page_size / DIR_ENTRY_SIZE;
   heapFile = fopen(argv[1], "r");
   init_heapfile(&heapfile, page_size, heapFile);  

   fseek(heapfile.file_ptr, 0, SEEK_SET);
   init_fixed_len_page(&dataPage, page_size, slot_size);
   init_directory_page(&dirPage, page_size); 

   // read in the first directory page
   fread(dirPage.data, page_size, 1, heapFile);
   
   // get the pointer that points to the next directory oage
   memcpy((void *) &nextDirPointer, dirPage.data, DIR_ENTRY_SIZE);
   
   do
   {
      for (int i = 1; i < capacity; i++)
      {
        memcpy((void *) &dirEntry, dirPage.data + DIR_ENTRY_SIZE * i, DIR_ENTRY_SIZE);
        if (dirEntry.page_offset != 0)
        {
          // read a data page out
          read_page(&heapfile, dirEntry.pid, &dataPage);
          for (int i = 0; i < slot_size; i++)
          {
             Record record;
             // read each record out
             read_fixed_len_page(&dataPage, i, &record, 1000);
             if (strncmp(record.at(0), empty, 10))
             {
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
          }
        }
      }
   
     // will be used as the condition for do-while loop
     dirOffset = nextDirPointer.page_offset;

     // There exists another directory page
     if (nextDirPointer.page_offset != 0)
     {
        fseek(heapFile, nextDirPointer.page_offset, SEEK_SET);
        init_directory_page(&dirPage, page_size); 
        fread(dirPage.data, page_size, 1, heapFile);
        memcpy((void *) &nextDirPointer, dirPage.data, DIR_ENTRY_SIZE);
     }
  } while (dirOffset != 0);

   fclose(heapFile);
   return 0;
}
