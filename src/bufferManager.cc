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

/**
 * Serialize the record to a byte array to be stored in buf.
 */
void fixed_len_write(Record *record, void *buf)
{
  for (Record::iterator i = record->begin(); i != record.end(); i++)
  {
    memcpy(buf, *i, 10);
    buf = (char *) buf + 10;
  }
}

/**
* Deserializes `size` bytes from the buffer, `buf`, and
* stores the record in `record`.
*/
void fixed_len_read(void *buf, int size, Record *record)
{
  for (int i = 0; i < size/10; i++)
  {
    memcpy(record->at(i), *buf, 10);
    buf = (char *) buf + 10;
  }
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
