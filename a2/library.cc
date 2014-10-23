#include "library.h"

static unsigned int page_size;

void mk_runs(FILE *in_fp, FILE *out_fp, long run_length, Schema *schema)
{
  int record_size = calc_record_size(schema);
  int buf_size = run_length * page_size;
  char *buf = malloc(buf_size);
  
  fread(buf, buf_size, 1, in_fp);
  qsort(buf, (page_size / record_size) * run_length , buf_size, compare_records);

  fwrite(buf, buf_size, 1, out_fp);
  free(buf);
}

void merge_runs(RunIterator* iterators[], int num_runs, FILE *out_fp,
                long start_pos, char *buf, long buf_size)
{
  // Your implementation
}

int calc_record_size(Schema *schema)
{
   int length = 0, i = 0;
   Attribute * attr = schema->attrs;
   
   while (attr != NULL) {
      length += attr->length;
      attr = schema->attrs[++i];
   }
   return length;
}

int compare_records(Record *r1, Record *r2) {
  Schema * schema = r1->schema;

  int i, offset, length, type;
  int grade = 0;
  void * buff1, buff2;
  
  for (i = 0; i < n_sort_attrs; i++)
  {
    for (j = 0; j < schema->sort_attrs[i]; j++)
    {
      offset += schema->attrs[j]->length;
      length = schema->attrs[j + 1]->length;
      type = schema->attrs[j + 1]->type;
    }

    buff1 = malloc(length);
    memset(buff1, 0, length);
    buff2 = malloc(length);
    memset(buff2, 0, length);
    char * data1 = (char *) r1->data + offset;
    char * data2 = (char *) r2->data + offset;
    memcpy(buff1, data1, length);
    memcpy(buff2, data2, length);

    if (type == ty.INTEGER)
    {
       if ((*(int *) data1) > (*(int *) data2))
       {
          free(buff1);
          free(buff2);
          return -1;
       }
       else if ((*(int *) data1) < (*(int *) data2))
       {
          free(buff1);
          free(buff2);
          return 1;
       }
    }

    if (type == ty.FLOAT)
    {
      if ((*(float *) data1) > (*(float *) data2))
      {
         free(buff1);
         free(buff2);
         return -1;
      }
      else if ((*(float *) data1) < (*(float *) data2))
      {
         free(buff1);
         free(buff2);
         return 1;
      }
    }

    if (type == ty.STRING)
    {
      int rc = strncmp(data1, data2, length);
      if (rc > 0)
      {
        free(buff1);
        free(buff2);
        return -1;
      }
      else if ( rc < 0)
      {
        free(buff1);
        free(buff2);
        return 1;
      }
    }
   
    free(buff1);
    free(buff2);
  }

  return 0;
}
