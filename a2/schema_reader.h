#include <stdlib.h>     /* qsort */
#include <iostream> 
#include <fstream> 
#include <json/json.h>
#include <cstring>
#include <sstream>

enum Types {INTEGER = 0, STRING = 1, FLOAT = 2};

typedef struct {
	std::string name;
	int length;
	int type;
} Attribute;

typedef struct {
	Attribute **attrs;
    int nattrs;
	int *sort_attrs;
	int n_sort_attrs;
} Schema;

class SchemaReader{
private:
	std::ifstream* schema_file;
    Schema* schema;
    Attribute* createAttribute(std::string name, int len, int type);
	Schema* read();
    void deallocate();
public:
	SchemaReader(std::string);
	~SchemaReader();
    Schema* getSchema();
    void addSortingAttributes(std::string csvAttrList);
	int getRecordSize();
	void serialize(std::string csvstring, char* data);
	void deserialize(char* in, std::string out);
};

class ExternalSorter {
private:
  SchemaReader* reader;
  std::ifstream csv_file;
  std::ofstream page_file;
  int mem_capacity; 
  void* mem;
  int record_size;

  int compareRecord(const void* rec1, const void* rec2); // for use by qsort()
  
public:
  ExternalSorter(std::string schema_filename);
  ~ExternalSorter();
  void setMemCapacity(int cap);
  void addSortingAttributes(std::string attrList);
  void csv2pagefile(std::string csv_filename, std::string pagefilename);  
};
