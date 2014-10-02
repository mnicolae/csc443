#include "heapManager.h"
#include "pageManager.h"

/**
 * Initialize a heapfile to use the file and page size given.
 */
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file)
{
   Page dir;
   PageID dirID = 0;

   heapfile->file_ptr = file;
   heapfile->page_size = page_size;

   init_directory_page(dir, page_size);
   write_page(&dir, heapfile, dirID); 
}

/** 
* Write a directory entry into a directory page.
*/
void write_dirEntry(Page *page, int entry_offset, DirectoryEntry *dirEntry)
{
//  DirectoryEntry dEntry;
//  d->Entry 
//  char * entry_offset = (char *) page->data + entry_offset * DIR_ENTRY_SIZE;
//
//  memcpy(entry_offset, buf, RECORD_SIZE);
//  free(buf);
}   

/**
 * Allocate another page in the heapfile. This grows the file by a page.
 */
PageID alloc_page(Heapfile *heapfile)
{
  Page dir;
  PageID dirID = 0;

  read_page(heapfile, dirID, dir);
 
  fread(page.data, 1, page.page_size, pageStream);
}
