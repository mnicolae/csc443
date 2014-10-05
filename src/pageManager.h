#include "bufferManager.h"

typedef struct {
void *data;
int page_size;
int slot_size;
} Page;

#define PAGE_STRUCT_SIZE 12

/***
* Calculate the slot size for a page given the page size
* and the record size
*/
int calculate_slot_size(int page_size);

/**
* Initializes a page using the given slot size
*/
void init_fixed_len_page(Page *page, int page_size, int slot_size);

/**
* Calculates the maximal number of records that fit in a page
*/
int fixed_len_page_capacity(Page *page);
 
/**
* Calculate the free space (number of free slots) in the page
*/
int fixed_len_page_freeslots(Page *page);
 
/**
* Add a record to the page
* Returns:
* record slot offset if successful,
* -1 if unsuccessful (page full)
*/
int add_fixed_len_page(Page *page, Record *r);

/**
* Delete a record from the page
* Returns:
* record 0 if successful,
* -1 if unsuccessful (slot is already empty)
*/
int delete_fixed_len_page(Page *page, Record *r, int slot_number);

/**
* Write a record into a given slot.
*/
void write_fixed_len_page(Page *page, int slot, Record *r);
 
/**
* Read a record from the page from a given slot.
*/
void read_fixed_len_page(Page *page, int slot, Record *r);
