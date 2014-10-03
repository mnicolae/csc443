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
  PageID result;
  int page_size = heapfile->page_size;

  Page * page;
  int slot_size = calculate_slot_size(page_size);
  init_fixed_len_page(page, page_size, slot_size);

  read_page(heapfile, dirID, &dir);
  PageID availDir = findAvailDir(&dir, page_size);
  if (availDir == 0)
  {
    result = findDirSlot(&dir, page_size);
    addDirctory(&dir, page_size)
    write_page(page, heapfile, result);
  }
  else
  {
    dir = loadDir(heapfile, availDir); 
    result = findDirslot(&dir, page_size);
    addDirctory(&dir, page_size);
    write_page(page, heapfile,  
    
  Page availDir
  read_page(heapfile, 

  return freePage;
  
}

/**
 * Read a page into memory
 */
void read_page(Heapfile *heapfile, PageID pid, Page *page)
{
  //off_t off_set = pid * heapfile->page_size;
  int off_set = getOffSet(heapfile, pid);
  if (off_set != -1)
  {
    fseeko(heapfile->file_ptr, offset, SEEK_SET);
    fread(page->data, 1, heapfile->page_size, heapfile->file_ptr);
  }
}

/**
 * Write a page from memory to disk
 */
void write_page(Page *page, Heapfile *heapfile, PageID pid)
{
  int off_set = getOffSet(heapfile, pid)l
  if (off_set != -1)
  {
    updateDirectory(heapfile, pid);
    fseeko(heapf, off_set, SEEK_SET);
    fwrite(page->data, page_size, 1, heapf);
    fflush(heapf);
  }
}

void updateDirectory(Heapfile * heapfile, PageID pid)
{
  File * heapf = heapfile->file_ptr;
  int page_size = heapfile->page_size;
  Page dirPage;
  init_fixed_len_page(&dirPage, page_size, 0);
  fread(dirPage->data, 1, page_size, heapf);

  int capacity = dirPage->page_size / DIR_ENTRY_SIZE;
  fread(dirPage->data, 1, heapfile->page_size, heapf);

  DirectoryEntry nextDir;
  memcpy(nextDir, dirPage->data, DIR_ENTRY_SIZE);
  DirectoryEntry dirEntry;

  do{ 
    for (int i = 1; i < capacity; i++)
    {
      void * buf = dirPage->data;
      memcpy(dirEntry, buf, DIR_ENTRY_SIZE);
      if (dirEntry.pid == pid)
      {
        dirEntry.freespace -= 1;
        memcpy(dirPage->data + i * DIR_ENTRY_SIZE, dirEntry, DIR_ENTRY_SIZE);
        fwrite(dirPage->data, 1, page_size, heapf);
      }
      buf = (char *) buf + 10;
    }

    fseeko(heapfile->file_ptr, nextDir.offset, SEEK_SET);
    fread(dirPage->data, 1, heapfile->page_size, heapf);
    memcpy(nextDir, dirPage->data, DIR_ENTRY_SIZE);

  } while (nextDir->offset != 0)
}


int getOffSet(Heapfile * heapfile, PageID pid)
{
  File * heapf = heapfile->file_ptr;
  Page dirPage;
  init_fixed_len_page(&dirPage, heapfile->page_size, 0); 
  fread(dirPage->data, 1, heapfile->page_size, heapf);

  int capacity = dirPage->page_size / DIR_ENTRY_SIZE;
  DirectoryEntry nextDir;
  memcpy(dirEntry, dirPage->data, DIR_ENTRY_SIZE);
  DirectoryEntry dirEntry;

  do{
    for (int i = 1; i < capacity; i++)
    {
      memcpy(dirEntry, dirPage->data, DIR_ENTRY_SIZE);
      if (dirEntry.pid == pid)
      {
        return dirEntry.offset;
      }
    }

    fseeko(heapfile->file_ptr, nextDir.offset, SEEK_SET);
    fread(dirPage->data, 1, heapfile->page_size, heapf);
    memcpy(nextDir, dirPage->data, DIR_ENTRY_SIZE);

  } while (nextDir->offset != 0)

  return -1;
}

getEmptySlot()
{
}


  
