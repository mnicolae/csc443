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
public:
	SchemaReader(std::string);
	~SchemaReader();
    Schema* getSchema();
    void addSortingAttributes(std::string csvAttrList);
    void deallocate();
};