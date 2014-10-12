#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "heapManager.h"

/**
 * Initialize a heapfile to use the file and page size given.
 */
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file)
{
   Page dir;
   PageID dirID = 0;

   heapfile->file_ptr = file;
   heapfile->page_size = page_size;
}

/**
 * Allocate another page in the heapfile. This grows the file by a page.
 */
PageID alloc_page(Heapfile *heapfile, int record_size)
{
   int page_size = heapfile->page_size;   
   FILE * heapf = heapfile->file_ptr;
   Page dirPage, dataPage;
   int dirPageCount = 0;
   int capacity = page_size / DIR_ENTRY_SIZE;
   int pageSlotSize = calculate_slot_size(page_size, record_size); 
   DirectoryEntry dirEntry, nextDirPointer;
   int dirOffset = 0;
   int oldOffset = 0;
   init_fixed_len_page(&dataPage, page_size, pageSlotSize);
   init_directory_page(&dirPage, heapfile->page_size, record_size); 
   fseek(heapf, 0, SEEK_SET);
   fread(dirPage.data, heapfile->page_size, 1, heapf);

   memcpy((void *) &nextDirPointer, dirPage.data, DIR_ENTRY_SIZE);

   do {
     // Current directory page has free entries
     if (nextDirPointer.freespace > 0)
     {
         for (int i = 1; i < capacity; i++)
         {
           memcpy((void *) &dirEntry, dirPage.data + DIR_ENTRY_SIZE * i, DIR_ENTRY_SIZE);
           if (dirEntry.page_offset == 0)
           {
             dirEntry.pid = dirPageCount * (capacity - 1) + i;
             dirEntry.page_offset = (dirEntry.pid + dirPageCount) * page_size;
             dirEntry.freespace = calculate_slot_size(page_size, RECORD_SIZE);

             nextDirPointer.freespace--; 
             memcpy(dirPage.data, (const void *) &nextDirPointer, DIR_ENTRY_SIZE);
             memcpy(dirPage.data + i * DIR_ENTRY_SIZE, (const void *) &dirEntry, DIR_ENTRY_SIZE);

             fseek(heapf, dirOffset, SEEK_SET);
             fwrite(dirPage.data, page_size, 1, heapf);
             fflush(heapf);

             return dirEntry.pid;
           }
         }
     }
     
     // will be used as the condition for do-while loop
     dirOffset = nextDirPointer.page_offset;

     // There exists another directory page
     if (nextDirPointer.page_offset != 0)
     {
        fseek(heapfile->file_ptr, nextDirPointer.page_offset, SEEK_SET);
        oldOffset = nextDirPointer.page_offset; 
        init_directory_page(&dirPage, heapfile->page_size, record_size); 
        fread(dirPage.data, heapfile->page_size, 1, heapf);
        memcpy((void *) &nextDirPointer, dirPage.data, DIR_ENTRY_SIZE);
        dirPageCount++;
     }
  } while (dirOffset != 0);
 
   // Allocate a new directory page. All other ones are full.
   
   // Write the old directory page to disk. It's offset is no longer 0.
   nextDirPointer.page_offset =  (1 + dirPageCount) * capacity * page_size;
   memcpy(dirPage.data, (const void *) &nextDirPointer, DIR_ENTRY_SIZE);
   fseek(heapf, oldOffset, SEEK_SET);
   fwrite(dirPage.data, page_size, 1, heapf);

   // create a new directory page and a new directory entry pointer
   Page newDirPage;
   DirectoryEntry newDirPagePointer;
   init_directory_page(&newDirPage, page_size, record_size);

   //get the first dir entry in the dir page, modify it to be a dir entry pointer and copy it back
   memcpy((void *) &newDirPagePointer, newDirPage.data, DIR_ENTRY_SIZE);
   newDirPagePointer.freespace--;
   memcpy(newDirPage.data, (const void *) &newDirPagePointer, DIR_ENTRY_SIZE);
   
   //get and modify the new dir entry and copy it back
   memcpy((void *) &dirEntry, newDirPage.data + DIR_ENTRY_SIZE, DIR_ENTRY_SIZE);
   dirEntry.page_offset = (1 + dirPageCount) * capacity * page_size + page_size;
   dirEntry.pid = (1 + dirPageCount) * (capacity - 1) + 1;
   dirEntry.freespace = page_size / RECORD_SIZE;

   memcpy(newDirPage.data + DIR_ENTRY_SIZE, (const void *) &dirEntry, DIR_ENTRY_SIZE);

   // copy the page back to the heap file
   fseek(heapf, nextDirPointer.page_offset, SEEK_SET); 
   fwrite(newDirPage.data, page_size, 1, heapf);
   fflush(heapf);
   
   return dirEntry.pid;
}

/**
 * Read a page into memory
 */
void read_page(Heapfile *heapfile, PageID pid, Page *page, int record_size)
{
  int off_set = getOffSet(heapfile, pid, record_size);
  if (off_set != -1)
  {
    fseek(heapfile->file_ptr, off_set, SEEK_SET);
    fread(page->data, 1, heapfile->page_size, heapfile->file_ptr);
  }
}

/**
 * Write a page from memory to disk
 */
void write_page(Page *page, Heapfile *heapfile, PageID pid, int record_size)
{
  FILE * heapFile = heapfile->file_ptr;
  int page_size = heapfile->page_size;
  int off_set = getOffSet(heapfile, pid, record_size);
  if (off_set != -1)
  {
    fseek(heapFile, off_set, SEEK_SET);
    fwrite(page->data, page_size, 1, heapFile);
    fflush(heapFile);
  }
}

/**
 * Get the offset of a Page in the heap file given its pid.
 */
int getOffSet(Heapfile * heapfile, PageID pid, int record_size)
{
  FILE * heapf = heapfile->file_ptr;
  Page dirPage;
  int dirOffset = 0;

  init_directory_page(&dirPage, heapfile->page_size, record_size); 
  fseek(heapf, 0, SEEK_SET);
  fread(dirPage.data, heapfile->page_size, 1, heapf);

  int capacity = dirPage.page_size / DIR_ENTRY_SIZE;
  DirectoryEntry dirEntry, nextDir;
  memcpy((void *) &nextDir, dirPage.data, DIR_ENTRY_SIZE);

  do {
    for (int i = 1; i < capacity; i++)
    {
      memcpy((void *) &dirEntry, dirPage.data + DIR_ENTRY_SIZE * i, DIR_ENTRY_SIZE);
      if (dirEntry.pid == pid)
      {
        return dirEntry.page_offset;
      }
    }

    dirOffset = nextDir.page_offset;

    if (dirOffset != 0)
    {
      fseek(heapfile->file_ptr, nextDir.page_offset, SEEK_SET);
      init_directory_page(&dirPage, heapfile->page_size, record_size); 
      fread(dirPage.data, heapfile->page_size, 1, heapf);
      memcpy((void *) &nextDir, dirPage.data, DIR_ENTRY_SIZE);
    }

  } while (dirOffset != 0);

  return -1;
}

/**
 * Update the freespace of a page in the heap file given its pid.
 */
void updateDirEntry(Heapfile * heapfile, PageID pid, int diff, int record_size)
{
  FILE * heapf = heapfile->file_ptr;
  int page_size = heapfile->page_size;
  Page dirPage;
  int dirOffset = 0;

  init_directory_page(&dirPage, heapfile->page_size, record_size); 
  fseek(heapf, 0, SEEK_SET);
  fread(dirPage.data, heapfile->page_size, 1, heapf);

  int capacity = dirPage.page_size / DIR_ENTRY_SIZE;
  DirectoryEntry dirEntry, nextDir;
  memcpy((void *) &nextDir, dirPage.data, DIR_ENTRY_SIZE);

  do {
    for (int i = 1; i < capacity; i++)
    {
      memcpy((void *) &dirEntry, dirPage.data + DIR_ENTRY_SIZE * i, DIR_ENTRY_SIZE);
      if (dirEntry.pid == pid)
      {
        dirEntry.freespace += diff;
        memcpy(dirPage.data + i * DIR_ENTRY_SIZE, (const void *) &dirEntry, DIR_ENTRY_SIZE);

        fseek(heapf, dirOffset, SEEK_SET);
        fwrite(dirPage.data, page_size, 1, heapf);
        fflush(heapf);
        
        return;
      }
    }

    dirOffset = nextDir.page_offset;

    if (dirOffset != 0)
    {
      fseek(heapfile->file_ptr, nextDir.page_offset, SEEK_SET);
      init_directory_page(&dirPage, heapfile->page_size, record_size); 
      fread(dirPage.data, heapfile->page_size, 1, heapf);
      memcpy((void *) &nextDir, dirPage.data, DIR_ENTRY_SIZE);
    }

  } while (dirOffset != 0);
}

/**
 * Find the first page in the heap file that can fit a new record.
 */
PageID findAvailablePage(Heapfile * heapfile, int record_size)
{
  FILE * heapf = heapfile->file_ptr;
  int page_size = heapfile->page_size;
  Page dirPage;
  int dirOffset = 0;

  init_directory_page(&dirPage, page_size, record_size); 
  fseek(heapf, 0, SEEK_SET);
  fread(dirPage.data, heapfile->page_size, 1, heapf);

  int capacity = dirPage.page_size / DIR_ENTRY_SIZE;
  DirectoryEntry dirEntry, nextDir;
  memcpy((void *) &nextDir, dirPage.data, DIR_ENTRY_SIZE);

  do {
    for (int i = 1; i < capacity; i++)
    {
      memcpy((void *) &dirEntry, dirPage.data + DIR_ENTRY_SIZE * i, DIR_ENTRY_SIZE);
      if (dirEntry.freespace > 0 && dirEntry.pid != 0)
      {
          return dirEntry.pid;
      }
    }

    dirOffset = nextDir.page_offset;

    if (dirOffset != 0)
    {
      fseek(heapfile->file_ptr, nextDir.page_offset, SEEK_SET);
      init_directory_page(&dirPage, heapfile->page_size, record_size); 
      fread(dirPage.data, heapfile->page_size, 1, heapf);
      memcpy((void *) &nextDir, dirPage.data, DIR_ENTRY_SIZE);
    }

  } while (dirOffset != 0);

  return -1;
}

/**
* Initializes a heap file directory page
*/
void init_directory_page(Page *page, int page_size, int record_size)
{
   page->page_size = page_size;
   page->slot_size = fixed_len_dir_capacity(page);

   page->data = new char[page_size];
   memset(page->data, 0, page_size);
   
   char * entry_offset = (char *) page->data;
   DirectoryEntry dirEntry, nextDirPointer;
   dirEntry.pid = 0;
   dirEntry.page_offset = 0;
   dirEntry.freespace = calculate_slot_size(page_size, record_size);
   nextDirPointer.page_offset = 0;
   nextDirPointer.pid = 0;
   nextDirPointer.freespace = page_size/DIR_ENTRY_SIZE - 1;
   memcpy(entry_offset, &nextDirPointer, DIR_ENTRY_SIZE);
   entry_offset += DIR_ENTRY_SIZE;

   for (int i=1; i < page->slot_size; i++)
   { 
       memcpy(entry_offset, &dirEntry, DIR_ENTRY_SIZE);
       entry_offset += DIR_ENTRY_SIZE;
   }
   
}

/**
* Calculates the maximal number of entries that fit in a
* directory page
*/
int fixed_len_dir_capacity(Page *page)
{
   return page->page_size / DIR_ENTRY_SIZE;
}
