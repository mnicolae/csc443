#include "schema_reader.h" 

Attribute* SchemaReader::createAttribute(const char* name, int len, int type) {
    Attribute* attr = new Attribute;
    attr->name = (char*) malloc(strlen(name));
    strcpy(attr->name, name);
    attr->length = len;
    attr->type = type;
    return attr;
}

SchemaReader::SchemaReader(char* filename) {
	schema_file = new std::ifstream(filename, std::ifstream::binary);
    schema = read();
    schema_file->close();
    delete schema_file;
}

SchemaReader::~SchemaReader() { /* do nothing */}

void SchemaReader::deallocate() { 
    // de-allocate Attributes and Schema
	Attribute* attr;
    for(int i=0; i < schema->nattrs; i++) {
    	attr = schema->attrs[i];
    	free(attr->name);
    	delete attr;
    }
    delete schema;
}

Schema* SchemaReader::read() {
  Json::Value root;
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(*schema_file, root, false);

  if (!parsingSuccessful) {
    std::cout << "reading json file failed\n" ;
  }

  Schema* retval = new Schema;
  std::string type, name;
  int length;

  int attr_count = root.size();
  retval->nattrs = attr_count;
  retval->attrs = new Attribute*[attr_count];

  for (int i=0; i < attr_count; i++) {
	name = root[i].get("name", "").asString();  
	type = root[i].get("type", "string").asString();
	length = root[i].get("length","").asInt();
	
	if (type == "string") {
		retval->attrs[i] = createAttribute(name.c_str(), length, STRING);
	}
	if (type == "integer") {
		retval->attrs[i] = createAttribute(name.c_str(), length, INTEGER);
	}
	if (type == "float") {
		retval->attrs[i] = createAttribute(name.c_str(), length, FLOAT);
	}
  }
  
  return retval;

}

Schema* SchemaReader::getSchema() {
	return schema;
}

//
// /* g++ -Wall -I. -g3 -o reader schema_reader.cpp jsoncpp.o */
//
//int main() {
//	SchemaReader reader("schema_example.json");
//  Schema* s = reader.getSchema(); // get a pointer to the schema
//	reader.deallocate(); // free memory that "s" occupies
//	return 0;
//}