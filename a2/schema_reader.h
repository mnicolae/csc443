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
	void deserialize(char* in, Record * record);
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
  ExternalSorter(std::string schema_filename);
  ~ExternalSorter();
  void setMemCapacity(int cap);
  void addSortingAttributes(std::string attrList);
  void csv2pagefile(std::string csv_filename, std::string pagefilename);  
};
