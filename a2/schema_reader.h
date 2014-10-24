#include <iostream> 
#include <fstream> 
#include <json/json.h>
#include <cstring>

enum Types {INTEGER = 0, STRING = 1, FLOAT = 2};

typedef struct {
	char* name;
	int length;
	int type;
} Attribute;

typedef struct {
	Attribute** attrs;
    int nattrs;
	int sort_attrs[];
	int n_sort_attrs;
} Schema;

class SchemaReader{
private:
	std::ifstream* schema_file;
    Schema* schema;
    Attribute* createAttribute(const char* name, int len, int type);
	Schema* read();
public:
	SchemaReader(char* filename);
	~SchemaReader();
    Schema* getSchema();
    void deallocate();
};