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

/* Update one attribute of a single record in the heap file given its record ID
 * <attribute_id> is the index of the attribute to be updated 
 * (e.g. 0 for the first attribute, 1 for the second attribute, etc.)
 * <new_value> will have the same fixed length (10 bytes)
 */
int main(int argc, char *argv[])
{
  char * newValue;
  int slotNumber, attrID, page_size, slot_size;
  Record record;
  Page dataPage;
  Heapfile hFile;
  FILE * heapFile;
  PageID pid;

  if (argc != 7)
  {
    printf("Usage: update <heapfile> <record_id> <attribute_id> <new_value> <page_size>\n");
  }
  
  heapFile = fopen(argv[1], "r+");
  pid = atoi(argv[2]);
  slotNumber = atoi(argv[3]);
  attrID = atoi(argv[4]);
  newValue = argv[5];
  page_size = atoi(argv[6]);

  init_heapfile(&hFile, page_size, heapFile);

  slot_size = calculate_slot_size(page_size, RECORD_SIZE);
  assert(page_size > RECORD_SIZE + PAGE_STRUCT_SIZE);

  init_fixed_len_page(&dataPage, page_size, slot_size); 

  // get the page that has the pid, read out the record that is in slot slotNumber
  read_page(&hFile, pid, &dataPage);
  read_fixed_len_page(&dataPage, slotNumber, &record, RECORD_SIZE);

  // udpate the record's attribute value
  record[attrID] = newValue;
  
  // write tha changes back to heapfile
  write_fixed_len_page(&dataPage, slotNumber, &record, RECORD_SIZE);
  write_page(&dataPage, &hFile, pid);

  fclose(heapFile);

  return 0;

}
