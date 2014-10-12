#include <iostream>
#include <stdio.h>
#include <vector>
#include "pageManager.h"

typedef struct {
FILE *file_ptr;
int page_size;
} Heapfile;

typedef int PageID;
 
typedef struct {
int page_id;
int slot;
} RecordID;

class RecordIterator {
    public:
    RecordIterator(Heapfile *heapfile);
    Record next();
    bool hasNext();
};

typedef struct {
    PageID pid;
    int page_offset;
    int freespace;
} DirectoryEntry;

#define DIR_ENTRY_SIZE sizeof(DirectoryEntry)

/**
 * Initialize a heapfile to use the file and page size given.
 */
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file);

/**
 * Allocate another page in the heapfile.  This grows the file by a page.
 */
PageID alloc_page(Heapfile *heapfile, int record_size);

/**
 * Read a page into memory
 */
void read_page(Heapfile *heapfile, PageID pid, Page *page, int record_size);

/**
 * Write a page from memory to disk
 */
void write_page(Page *page, Heapfile *heapfile, PageID pid, int record_size);

/**
 * Get the offset of a Page in the heap file given its pid.
 */
int getOffSet(Heapfile * heapfile, PageID pid, int record_size);

/**
 * Update the freespace of a page in the heap file given its pid.
 */
void updateDirEntry(Heapfile * heapfile, PageID pid, int diff, int record_size);

/**
 * Find the first page in the heap file that can fit a new record.
 */
PageID findAvailablePage(Heapfile * heapfile, int record_size);

/**
* Initializes a heap file directory page
*/
void init_directory_page(Page *page, int page_size, int record_size);

/**
* Calculates the maximal number of entries that fit in a
* directory page
*/
int fixed_len_dir_capacity(Page *page);
