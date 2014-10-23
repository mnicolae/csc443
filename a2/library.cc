#include "library.h"

static unsigned int page_size;

void mk_runs(FILE *in_fp, FILE *out_fp, long run_length, Schema *schema)
{
  int record_size = calc_record_size(schema);
  int buf_size = run_length * page_size;
  char *buf = malloc(buf_size);
  
  fread(buf, buf_size, 1, in_fp, compare_records);
  qsort(buf, (page_size / record_size) * run_length , buf_size, compare_records);
 
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

}
