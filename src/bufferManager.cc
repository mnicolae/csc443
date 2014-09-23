#include "bufferManager.h"
#include <vector>
#include <stdio.h>
#include <string.h>

/**
 * Compute the number of bytes required to serialize record
 */
int fixed_len_sizeof(Record *record)
{
  int numBytes = 0;
  int size = record->size();
  for (int i = 0; i < size; i++)
  {
    numBytes += strlen(record->at(i));
  }
  return numBytes;
}






















/*int main()
{
  int size;
  std::vector<int> myvec (10);
  for (int i = 0; i < myvec.size(); i++)
  {
    myvec.at(i) = i;
  }

  size = fixed_len_sizeof(myvec);
  printf("%d", size);
  return 0;
}*/
