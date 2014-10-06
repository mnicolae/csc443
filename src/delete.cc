#include <stdio.h>
#include <fstream>
#include <string.h>
#include <vector>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <assert.h>
#include "heapManager.h"

// Delete a single record in the heap file given its record ID
int main(int argc, char *argv[])
{
  char * newValue;
  int slotNumber, attrID, page_size, slot_size, rc;
  Record record;
  Page dataPage;
  Heapfile hFile;
  FILE * heapFile;
  PageID pid;
  bool * slot;
  char * empty = "          ";

  if (argc != 5)
  {
    printf("Usage: delete <heapfile> <record_id> <page_size>\n");
  }
  
  heapFile = fopen(argv[1], "r+");
  pid = atoi(argv[2]);
  slotNumber = atoi(argv[3]);
  page_size = atoi(argv[4]);

  init_heapfile(&hFile, page_size, heapFile);

  slot_size = calculate_slot_size(page_size);
  assert(page_size > RECORD_SIZE + PAGE_STRUCT_SIZE);

  init_fixed_len_page(&dataPage, page_size, slot_size); 

  // read page containing record to be deleted
  read_page(&hFile, pid, &dataPage);
  read_fixed_len_page(&dataPage, slotNumber, &record);

  slot = (bool *) dataPage.data + dataPage.page_size - dataPage.slot_size + slotNumber;
  if (!(*slot))
  {
    return -1;
  }

  for (int i = 0; i < 100; i++)
  {
    record[i] = empty;
  }
  
  // update data entry freespace info and data page slot info 
  updateDirEntry(&hFile, pid, -1); 
  *slot = false;
  
  // write back the updated data page 
  write_fixed_len_page(&dataPage, slotNumber, &record);
  write_page(&dataPage, &hFile, pid);

  fclose(heapFile);
  return 0;
}
