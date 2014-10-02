#include <stdio.h>
#include <fstream>
#include <string.h>
#include <vector>
#include <cstring>
#include <iostream>
#include <stdlib.h>
#include "pageManager.h"
#include "bufferManager.h"

int calculate_slot_size(int page_size)
{
   int page_capacity = page_size / RECORD_SIZE;
   int page_remainder = page_size - (page_capacity * RECORD_SIZE);
   
   if (page_remainder >= page_capacity)
   {
       return page_capacity;
   }
   else
   {
      return page_capacity - 1; 
   }
}

/**
* Initializes a page using the given slot size
*/
void init_fixed_len_page(Page *page, int page_size, int slot_size)
{
   //page = (Page *) malloc(page_size); 
   page->page_size = page_size;
   page->slot_size = slot_size;

   page->data = new char[page_size];
   memset(page->data, 0, page_size);
}

/**
* Initializes a heap file directory page
*/
void init_directory_page(Page *page, int page_size)
{
   page->page_size = page_size;
   page->slot_size = fixed_len_capacity(page);

   page->data = new char[page_size];
   memset(page->data, 0, page_size);
   
   char * entry_offset = page->data;
   DirectoryEntry dirEntry;
   dirEntry->page_offset = 0;
   dirEntry->freespace = 0;

   for (int i=0; i < page->slot_size; i++)
   { 
       memcpy(entry_offset, &dirEntry, DIR_ENTRY_SIZE);
       entry_offset += DIR_ENTRY_SIZE;
   }
}

/**
* Calculates the maximal number of records that fit in a page
*/
int fixed_len_page_capacity(Page *page)
{
   int pageSize = page->page_size;
   int slotSize = page->slot_size;

   return (pageSize - slotSize) / RECORD_SIZE;
}
 
/**
* Calculates the maximal number of entries that fit in a
* directory page
*/
int fixed_len_page_capacity(Page *page)
{
   return page->page_size / DIR_ENTRY_SIZE:
}

/**
* Calculate the free space (number of free slots) in the page
*/
int fixed_len_page_freeslots(Page *page)
{
   bool * slot;
   slot = (bool *) page->data + page->page_size - page->slot_size;
   int counter = 0;
   for (int i = 0; i < page->slot_size; i++)
   {
     if (slot)
     {
       counter += 1;
     }
     slot += 1;
   }
   return counter;
} 

/**
* Add a record to the page
* Returns:
* record slot offset if successful,
* -1 if unsuccessful (page full)
*/
int add_fixed_len_page(Page *page, Record *r)
{
  bool * slot;
  slot = (bool *) page->data + page->page_size - page->slot_size;
  int free_slot = -1;
  for (int i = 0; i < page->slot_size; i++)
  {
    if (!(*slot))
    {
      free_slot = i;
      *slot = true;
      break;
    }
    slot += 1;
  }

  if (free_slot != -1)
  {
    write_fixed_len_page(page, free_slot, r);
  }

  return free_slot;
}


/**
* Write a record into a given slot.
*/
void write_fixed_len_page(Page *page, int slot, Record *r)
{
  char * record_offset = (char *) page->data + slot * RECORD_SIZE;
  void * buf = malloc(RECORD_SIZE); // TODO delete malloc
  fixed_len_write(r, buf);
  memcpy(record_offset, buf, RECORD_SIZE);
  free(buf);
}

 
/**
* Read a record from the page from a given slot.
*/
void read_fixed_len_page(Page *page, int slot, Record *r)
{
  char * record_offset = (char *) page->data + slot * RECORD_SIZE;
  void * buf = record_offset;
  fixed_len_read(buf, RECORD_SIZE, r);
  int c = 0;
}



