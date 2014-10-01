#include <stdio.h>
#include <fstream>
#include <string.h>
#include "bufferManager.h"
#include "pageManager.h"
#include <vector>
#include <cstring>
#include <iostream>
#include <cstdlib>

using namespace std;

int main(int argc, char *argv[])
{

  char * delimeter = ",";
  if (argc != 4)
  {
    printf("Usage: write_fixed_len_pages <csv_file> <page_file> <page_size>\n");
  }
  
  ifstream csvStream;
  FILE * pageStream;

  csvStream.open(argv[1]);
  pageStream =fopen(argv[2], "w");

  int page_size = atoi(argv[3]);

  char line[1200];

  char * attr;
  Record record;
  char c;
  int numLines = 0;
  int slot_size = calculate_slot_size(page_size); 
  Page page;
  
  init_fixed_len_page(&page, page_size, slot_size); 

  int rc = 0;
  for (int i = 0; i < 10; i++) // TODO: fix this
  {
     attr = NULL;

     csvStream.getline(line, 1200);
   
     attr = strtok(line, delimeter);
   
     while (attr)
     {
       record.push_back(attr);
       attr = strtok(NULL, delimeter);
     }
     
     rc = add_fixed_len_page(&page, &record); 
     if (rc == -1)
     {
	fwrite(page.data, 1, page.page_size, pageStream);
        init_fixed_len_page(&page, page_size, slot_size); 
     }
     
     for (int i = 0; i < 100; i++)
     {
       printf("%s\n", record.at(i));
     }
     
  }

  fwrite(page.data, 1, page.page_size, pageStream);

  csvStream.close();
  fclose(pageStream);

  return 0;
}
