#include <stdio.h>
#include <fstream>
#include <string.h>
#include <vector>
#include <cstring>
#include <iostream>
#include <stdlib.h>
#include "pageManager.h"

/**
* calculate the slot size for a page
**/
int calculate_slot_size(int page_size, int recordSize)
{
   int page_capacity = page_size / recordSize;
   int page_remainder = page_size - (page_capacity * recordSize);
   
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
   page->page_size = page_size;
   page->slot_size = slot_size;

   page->data = new char[page_size];
   memset(page->data, 0, page_size);
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
* Calculate the free space (number of free slots) in the page
*/
int fixed_len_page_freeslots(Page *page)
{
   bool * slot;
   slot = (bool *) page->data + page->page_size - page->slot_size;
   int counter = 0;

   //iterate through a page's slots, check for free slots
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

  // iterate through slots of a page, check for a free slot
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

  // add the record to the free slot
  if (free_slot != -1)
  {
    write_fixed_len_page(page, free_slot, r);
  }

  return free_slot;
}

/**
* Delete a record from the page
* Returns:
* record 0 if successful,
* -1 if unsuccessful (slot is already empty)
*/
int delete_fixed_len_page(Page *page, Record *r, int slot_number)
{
   bool * slot = (bool *) page->data + page->page_size - page->slot_size + slot_number; 

   Record record = *r;
   const char * empty = "          ";
   if (!(*slot))
   {
      return -1;
   }

   // replace the content of attributes of a record to empty content 
   for (int i=0; i < 100; i++)
   {
      record[i] = empty;
   }

   *slot = false;
   return 0;
}

/**
* Write a record into a given slot.
*/
void write_fixed_len_page(Page *page, int slot, Record *r)
{
  char * record_offset = (char *) page->data + slot * RECORD_SIZE;
  void * buf = malloc(RECORD_SIZE);
  fixed_len_write(r, buf);
  memcpy(record_offset, buf, RECORD_SIZE);
}

/**
* Read a record from the page from a given slot.
*/
void read_fixed_len_page(Page *page, int slot, Record *r)
{
  char * record_offset = (char *) page->data + slot * RECORD_SIZE;
  void * buf = record_offset;
  fixed_len_read(buf, RECORD_SIZE, r);
}
