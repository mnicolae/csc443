/**
* Initializes a page using the given slot size
*/
void init_fixed_len_page(Page *page, int page_size, int slot_size)
{
   page = (Page *) malloc(page_size);
   page->page_size = page_size;
   page->slot_size = slot_size;
   memset(page, 0, page_size);
}
 
/**
* Calculates the maximal number of records that fit in a page
*/
int fixed_len_page_capacity(Page *page);
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
   slot = page + page->page_size - page->slot_size;
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
  slot = page + page->page_size - page->slot_size;
  int free_slot = -1;
  for (int i = 0; i < page->slot_size; i++)
  {
    if (slot)
    {
      free_slot = slot;
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
  char * record_offset = page + slot * RECORD_SIZE;
  void buf = malloc(RECORD_SIZE);
  fixed_len_write(r, &buf);
  memcpy(record_offset, &buf, RECORD_SIZE);
  free(buf);
}

 
/**
* Read a record from the page from a given slot.
*/
void read_fixed_len_page(Page *page, int slot, Record *r)
{
  char * record_offset = page + slot * RECORD_SIZE;
  void buf = malloc(RECORD_SIZE);
  fixed_len_read(&buf, RECORD_SIZE, r);
  free(buf);
}



