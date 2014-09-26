#include <stdio.h>
#include <fstream>
#include <string.h>
#include "bufferManager.h"
#include <vector>
#include <cstring>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{

  char * delimeter = ",";
  if (argc != 4)
  {
    printf("Usage: write_fixed_len_pages <csv_file> <page_file> <page_size>\n");
  }
  
  ifstream csvStream;
  ofstream pageStream;

  csvStream.open(argv[1]);
  pageStream.open(argv[2]);

  int page_size = argv[3][0];

  char line[1200];
  csvStream.getline(line, 1200);

  char * attr;
  attr = strtok(line, delimeter);

  Record record;
  
  while (attr)
  {
    record.push_back(attr);
    attr = strtok(NULL, delimeter);
  }

  for (int i = 0; i < 100; i++)
  {
    printf("%s\n", record.at(i));
  }

  csvStream.close();
  pageStream.close();

  return 0;
}
