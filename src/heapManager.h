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
PageID alloc_page(Heapfile *heapfile);

/**
 * Read a page into memory
 */
void read_page(Heapfile *heapfile, PageID pid, Page *page);

/**
 * Write a page from memory to disk
 */
void write_page(Page *page, Heapfile *heapfile, PageID pid);

int getOffSet(Heapfile * heapfile, PageID pid);

/**
* Initializes a heap file directory page
*/
void init_directory_page(Page *page, int page_size);

int fixed_len_dir_capacity(Page *page);

