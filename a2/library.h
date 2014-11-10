#include <cstdio>
#include <stdlib.h>     /* qsort */
#include <cstdio>
#include <cstdlib>
#include <iostream> 
#include <fstream>
#include <sstream>
#include <cstring>
#include <time.h>
#include <math.h>
#include <sys/timeb.h>

#include "json/json.h"

enum Types {
	INTEGER = 0, STRING = 1, FLOAT = 2
};

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

class SchemaReader {
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
	std::fstream csv_file;
	std::fstream page_file;
	int mem_capacity;
	void* mem;
	int record_size;

public:
	ExternalSorter(SchemaReader* rdr, int mem_cap);
	ExternalSorter(std::string schema_filename);
	~ExternalSorter();
	void setMemCapacity(int cap);
	void addSortingAttributes(std::string attrList);
	// int csv2pagefile(std::string csv_file, std::fstream * page_file);
	int csv2pagefile(std::string csv_file, std::string page_file);
	SchemaReader* getSchemaReader();
};


/**
 * The iterator helps you scan through a run.
 * you can add additional members as your wish
 */
class RunIterator {
private:
	std::fstream * fp;
	long cur_pos; // current position in page file
	long start_pos;
	long run_length;
	long buf_size;
	SchemaReader *reader;
	int record_length;

	char* buffer;
	int cur_rec_pos; // current position in buffer

	int fillBuffer();

public:
	/**
	 * Creates an interator using the `buf_size` to
	 * scan through a run that starts at `start_pos`
	 * with length `run_length`.
	 */

	RunIterator(std::fstream * pagefile, long start_pos, long run_length,
			long buf_size, SchemaReader *sr);

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

	SchemaReader* getReader();
};

/**
 * Creates sorted runs of length `run_length` in
 * the `out_fp`.
 */
int mk_runs(char* csv_fn, std::fstream out_file, long run_length, SchemaReader *reader);


/**
 * Merge runs given by the `iterators`.
 * The number of `iterators` should be equal to the `num_runs`.
 * Write the merged runs to `out_fp` starting at position `start_pos`.
 * Cannot use more than `buf_size` of heap memory allocated to `buf`.
 */
int merge_runs(RunIterator* iterators[], int num_runs, std::fstream *out_fp,
		long start_pos, char *buf, long buf_size);

int compare_records(const void *rec1, const void *rec2);

