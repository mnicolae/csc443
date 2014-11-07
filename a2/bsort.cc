#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include "leveldb/db.h"
#include "json/json.h"
#include <time.h>
#include <sys/timeb.h>

#include "library.h"

using namespace std;

int main(int argc, const char* argv[]) {

  if (argc != 4) {
  	cout << "ERROR: invalid input parameters!" << endl;
  	cout << "Please enter <schema_file> <input_file> <out_index>" << endl;
  	exit(1);
  }

  string schema_file(argv[1]);
  int ch;
  int offset_counter;
  char * attr;
  const char * delimeter = ",";
  void * buf;
  ifstream csvStream;
  FILE * csvNumLines;
  int record_size;
  const char *out_index = argv[3];
  long unique_counter = 0;
  Record record;
  struct timeb start, end;
  int count = 0;
  FILE * fp;
  
  csvStream.open(argv[2]);
  csvNumLines = fopen(argv[2], "r");

  /**
  * opens a database connection to "./leveldb_dir"
  */
  leveldb::DB *db;
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, "./leveldb_dir", &db);

  SchemaReader reader(schema_file);
  record_size = reader.getRecordSize();
  buf = malloc(record_size);

  char key[record_size];
  char unique_key[record_size + sizeof(long)];

  Schema * schema = reader.getSchema();

  char line[record_size + schema->nattrs]; 
  char line_value[record_size + schema->nattrs]; 

  ftime(&start);
  long start_time = start.time * 1000 + start.millitm; 

  // Do work here

  // get the number of lines
  do
  {
      ch = fgetc(csvNumLines);
      if (ch == '\n') count++;
  } while (ch != EOF);
	
  // parse the records in the csv file
  for (int i = 0; i < count; i++)
  {
     csvStream.getline(line, record_size + schema->nattrs);

     // add the key
     strncpy(line_value, line, record_size + schema->nattrs);
     line_value[record_size + schema->nattrs] = '\0';
     leveldb::Slice value(line_value, record_size + schema->nattrs);

     attr = NULL;
     offset_counter = 0;

     attr = strtok(line, delimeter);

     for (int j = 0; j < schema->nattrs; j++)
     {
       memcpy(buf + offset_counter, attr, schema->attrs[j]->length);
       attr = strtok(NULL, delimeter);
       offset_counter += schema->attrs[j]->length;
     }

     // construct the record
     record.data = (char *) buf;
     record.data[record_size] = '\0'; // null-terminate the record data
     record.schema = schema;
     
     memcpy(unique_key, &record, record_size);
     memcpy(unique_key + record_size, &unique_counter, sizeof(long));
  
     unique_counter++;

     db->Put(leveldb::WriteOptions(), leveldb::Slice(unique_key, sizeof(unique_key)), value);
  }

  fp = fopen(out_index, "w+");

  // Index will be sorted in order of value, so a left-to-right traversal
  // yields the sorted order.
  leveldb::Iterator *it = db->NewIterator(leveldb::ReadOptions());
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
      leveldb::Slice value = it->value();
      fprintf(fp, "%s", value.ToString().c_str()); // No \n, since the \n is part of the record.
      fprintf(fp, "\n");
  }

  ftime(&end);
  long end_time = end.time * 1000 + end.millitm;  

  printf("\nTIME: %ld miliseconds\n", end_time - start_time);

  fclose(fp);
  csvStream.close();
  delete db;

  return 0;
}


