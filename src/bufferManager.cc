#include "bufferManager.h"
#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
void fixed_len_write(Record *record, void *buf, int attrNum)
{
  for (int i = 0; i < attrNum; i++)
  {
    memcpy(buf, record->at(i), 10);
    buf = (char *) buf + 10;
  }
}

/**
* Deserializes `size` bytes from the buffer, `buf`, and
* stores the record in `record`.
*/
void fixed_len_read(void *buf, int size, Record *record, int attrNum)
{
  char * attr;
  attr = (char *) malloc(11);

  for (int i = 0; i < attrNum; i++)
  {
    memcpy(attr, (const void *) buf, 10);
    attr[10] = '\0';
    record->push_back(attr);
    buf = (char *) buf + 10;
  }
  free(attr);
}
