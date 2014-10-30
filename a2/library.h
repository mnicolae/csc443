#include <cstdio>
#include <stdlib.h>     /* qsort */
#include <iostream> 
#include <fstream> 
#include "json/json.h"
#include <cstring>
#include <sstream>

enum Types {INTEGER = 0, STRING = 1, FLOAT = 2};

/**
 * An attribute schema. You should probably modify
 * this to add your own fields.
 */

typedef struct {
	std::string name;
	int length;
	int type;
} Attribute;

/**
 * A record schema contains an array of attribute
 * schema `attrs`, as well as an array of sort-by 
 * attributes (represented as the indices of the 
 * `attrs` array).
 */

typedef struct {
	Attribute **attrs;
    int nattrs;
	int *sort_attrs;
	int n_sort_attrs;
} Schema;

/**
 * A record can defined as a struct with a pointer
 * to the schema and some data. 
 */
typedef struct {
  Schema* schema;
  char* data;
} Record;



class SchemaReader{
private:
	std::ifstream* schema_file;
    Schema* schema;
    Attribute* createAttribute(std::string name, int len, int type);
	Schema* read();
    void deallocate();
public:
	SchemaReader(Schema * sm); 
	SchemaReader(std::string);
	~SchemaReader();
    Schema* getSchema();
    void addSortingAttributes(std::string csvAttrList);
	int getRecordSize();
	void serialize(std::string csvstring, char* data);
	void deserialize(char* data, Record *rec);
};

int compareRecord(const void* rec1, const void* rec2); // for use by qsort()

class ExternalSorter {
private:
  SchemaReader* reader;
  std::ifstream csv_file;
  std::ofstream page_file;
  int mem_capacity; 
  void* mem;
  int record_size;

  
public:
  ExternalSorter(Schema *sm, int mem_cap, std::string csv_fn, std::string page_fn);
  ExternalSorter(std::string schema_filename);
  ~ExternalSorter();
  void setMemCapacity(int cap);
  void addSortingAttributes(std::string attrList);
  void csv2pagefile(std::string csv_filename, std::string pagefilename);  
};

/**
 * Creates sorted runs of length `run_length` in
 * the `out_fp`.
 */
void mk_runs(FILE *in_fp, FILE *out_fp, long run_length, Schema *schema);

/**
 * The iterator helps you scan through a run.
 * you can add additional members as your wish
 */
class RunIterator {
private:
	std::ifstream *fp; 
	long cur_pos; // current position in page file
	long start_pos; 
	long run_length; 
	long buf_size;
    SchemaReader *reader;
	int record_length;

	void* buffer;
	int cur_rec_pos; // current position in buffer

	int fillBuffer();

public:
  /**
   * Creates an interator using the `buf_size` to
   * scan through a run that starts at `start_pos`
   * with length `run_length`.
   */
  
  RunIterator(std::ifstream *pagefile, long start_pos, long run_length, long buf_size,
              SchemaReader *sr);

  /**
   * free memory
   */
  ~RunIterator();

  /**
   * reads the next record
   */
  Record* next();

  /**
   * return false if iterator reaches the end
   * of the run
   */
  bool has_next();

  int get_number_of_records();
};

/**
 * Merge runs given by the `iterators`.
 * The number of `iterators` should be equal to the `num_runs`.
 * Write the merged runs to `out_fp` starting at position `start_pos`.
 * Cannot use more than `buf_size` of heap memory allocated to `buf`.
 */
void merge_runs(RunIterator* iterators[], int num_runs, FILE *out_fp,
                long start_pos, char *buf, long buf_size);

int calc_record_size(Schema *schema);
int compare_records(const void *rec1, const void *rec2);
