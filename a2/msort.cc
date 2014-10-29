#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

#include "library.h"
#include "json/json.h"

#include <string.h>
#include <time.h>
#include <sys/timeb.h>

using namespace std;

int main(int argc, const char* argv[]) {
  struct timeb start, end;
  long start_time, end_time;

  if (argc < 7) {
    cout << "ERROR: invalid input parameters!" << endl;
    cout << "Please enter <schema_file> <input_file> <output_file> <mem_capacity> <k> <sorting_attributes>" << endl;
    exit(1);
  }
  string schema_file(argv[1]);
  //string input_file(argv[2]);
  //string output_file(argv[3]);
  //int mem_capacity(argv[4]);
  //int k(argv[5]);
  //string sorting_attributes[];

  // Parse the schema JSON file
  Json::Value schema;
  Json::Reader json_reader;
  // Support for std::string argument is added in C++11
  // so you don't have to use .c_str() if you are on that.
  ifstream schema_file_istream(schema_file.c_str(), ifstream::binary);
  bool successful = json_reader.parse(schema_file_istream, schema, false);
  if (!successful) {
    cout << "ERROR: " << json_reader.getFormatedErrorMessages() << endl;
    exit(1);
  }

  // Print out the schema
  string attr_name;
  int attr_len;
  for (int i = 0; i < schema.size(); ++i) {
    attr_name = schema[i].get("name", "UTF-8" ).asString();
    attr_len = schema[i].get("length", "UTF-8").asInt();
    cout << "{name : " << attr_name << ", length : " << attr_len << "}" << endl;

    //memcpy(my_attribute.name, attr_name.c_str(), attr_name.length() + 1); 
  }
  
  // TODO: Parse the input CSV file

  ftime(&start);
  start_time = start.time * 1000 + start.millitm; 

  // Do the sort
  // Your implementation

  ftime(&end);
  end_time = end.time * 1000 + end.millitm;  

  printf("\nTIME: %ld miliseconds\n", end_time - start_time);

  return 0;
}

