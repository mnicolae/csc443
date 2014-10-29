#include "library.h"
#include <stdlib.h>
#include <string.h>

static unsigned int page_size;
Schema * SCHEMA;

Attribute* SchemaReader::createAttribute(std::string name, int len, int type) {
    Attribute* attr = new Attribute;
    attr->name.append(name);
    attr->length = len;
    attr->type = type;
    return attr;
}

SchemaReader::SchemaReader(Schema * sm) {
	schema = sm;
}

SchemaReader::SchemaReader(std::string filename) {
	schema_file = new std::ifstream(filename.c_str(), std::ifstream::binary);
    schema = read();
    schema_file->close();
    delete schema_file;
    
    // initialize the sorting attributes
    schema->n_sort_attrs = 0;
    schema->sort_attrs = new int[schema->nattrs];
}

SchemaReader::~SchemaReader() { 
  deallocate();
}

void SchemaReader::deallocate() { 
    // de-allocate Attributes and Schema
    for(int i=0; i < schema->nattrs; i++) {
    	delete schema->attrs[i];
    }
    delete schema;
}

Schema* SchemaReader::read() {
  Json::Value root;
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(*schema_file, root, false);

  if (!parsingSuccessful) {
    std::cout << "Failed to read schema.\nExitting...\n";
    exit(1);
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
		retval->attrs[i] = createAttribute(name, length, STRING);
	}
	
	if (type == "integer") {
		retval->attrs[i] = createAttribute(name, length, INTEGER);
	}
	
	if (type == "float") {
		retval->attrs[i] = createAttribute(name, length, FLOAT);
	}
  }
  
  return retval;

}

Schema* SchemaReader::getSchema() {
	return schema;
}

void SchemaReader::addSortingAttributes(std::string csvAttrList) {
	std::stringstream list(csvAttrList);
	std::string attr;
	std::vector<int> sort_indices;
	int i=0;
	int count=0;
	
	while (list.good()) {
		getline(list, attr, ',');
		if (attr.length() == 0) {
			continue;
		}
		count ++; 
		for(i=0; i < schema->nattrs; i++) {
			if (schema->attrs[i]->name == attr) {
				sort_indices.push_back(i);
			}
		}		
	}
	
	if (count > 0) {
		schema->n_sort_attrs = count;
		//schema->sort_attrs = new int[count];
		for(i=0; i < count; i++) {
			schema->sort_attrs[i] = sort_indices[i];
		}		
	}
}

int SchemaReader::getRecordSize() {
  int count = 0; 
  Attribute *attr;

  for(int i=0; i < schema->nattrs; i++) {
	attr = schema->attrs[i];

	if (attr->type == STRING) {
	  count += attr->length;
	} else if (attr->type == INTEGER) {
	  count += sizeof(int);
	} else if (attr->type == FLOAT) {
	  count += sizeof(float);
	}
  }	// end for

  return count;
}

void SchemaReader::serialize(std::string csvstring, char* data) {
  std::stringstream record_data(csvstring);
  std::string attr_val;
  int attr_idx = 0;
  Attribute *attr;
  char *ptr = data;

  while (record_data.good()) {
	getline(record_data, attr_val, ','); // read each attribute
	
	attr = schema->attrs[attr_idx];

	if (attr->type == STRING) {
	  memcpy(ptr, (void*) attr_val.c_str(), attr->length);
	  ptr += attr->length;
	} else if (attr->type == INTEGER) {
	  *(int*)ptr = atoi(attr_val.c_str());
	  ptr += sizeof(int);
	} else if (attr->type == FLOAT) {
	  *(float*)ptr = atof(attr_val.c_str());
	  ptr += sizeof(float);
	}

	attr_idx++;
  }
}

void SchemaReader::deserialize(char*data, Record* record) {
  // TODO
} 

////////////////////////////////////////////////////////////
//
//     External Sorter class   
//
////////////////////////////////////////////////////////////

ExternalSorter::ExternalSorter(Schema *sm, int mem_cap, std::string csv_fn, std::string page_fn) {
	//TODO
}

ExternalSorter::ExternalSorter(std::string schema_filename) {
  reader = new SchemaReader(schema_filename);
  mem_capacity = 3072; // default memory capacity is 3KB
  mem = malloc(mem_capacity);
  record_size = reader->getRecordSize();
  SCHEMA = reader->getSchema();
}

ExternalSorter::~ExternalSorter() {
  free(mem);
  delete reader;
}

void ExternalSorter::setMemCapacity(int cap) {
  free(mem);
  mem_capacity = cap;
  mem = malloc(mem_capacity);
}

void ExternalSorter::addSortingAttributes(std::string attrList) {
  reader->addSortingAttributes(attrList);
}

int compareRecord(const void* r1, const void* r2) {
  int i, j, offset, length, type;
  void * buff1, *buff2;

  for (i = 0; i < SCHEMA->n_sort_attrs; i++) {
    offset = 0;
    for (j = 0; j < SCHEMA->sort_attrs[i]; j++) {
      offset += SCHEMA->attrs[j]->length;
    }  //TODO; fix sorting on second attribute

   type = SCHEMA->attrs[i]->type;
   if (type == STRING) { 
	length = SCHEMA->attrs[i]->length;
   } else if (type == INTEGER) {
	length = sizeof(int);
   } else if (type == FLOAT) {
	length = sizeof(float);
   } 

    buff1 = malloc(length);
    memset(buff1, 0, length);
    buff2 = malloc(length);
    memset(buff2, 0, length);
    char * data1 = (char *) r1 + offset;
    char * data2 = (char *) r2 + offset;
    memcpy(buff1, data1, length);
    memcpy(buff2, data2, length);

    if (type == INTEGER)
    {
       if ((*(int *) data1) > (*(int *) data2))
       {
          free(buff1);
          free(buff2);
          return 1;
       }
       else if ((*(int *) data1) < (*(int *) data2))
       {
          free(buff1);
          free(buff2);
          return -1;
       }
    }

    if (type == FLOAT)
    {
      if ((*(float *) data1) > (*(float *) data2))
      {
         free(buff1);
         free(buff2);
         return 1;
      }
      else if ((*(float *) data1) < (*(float *) data2))
      {
         free(buff1);
         free(buff2);
         return -1;
      }
    }

    if (type == STRING)
    {
      int rc = strncmp(data1, data2, length);
      if (rc > 0)
      {
        free(buff1);
        free(buff2);
        return 1;
      }
      else if ( rc < 0)
      {
        free(buff1);
        free(buff2);
        return -1;
      }
    }
   
    free(buff1);
    free(buff2);
  }

  return 0;
}

void ExternalSorter::csv2pagefile(std::string csv_file, std::string page_file) {
  std::ifstream infile(csv_file.c_str(), std::ifstream::binary);
  std::ofstream outfile(page_file.c_str(), std::ofstream::binary);

	if (!infile.is_open()) {
	  std::cout << "unable to open csv file";
	  return;
	}

	// initialize the buffer
	memset(mem, 0, mem_capacity);

	// calculate how many records will fit in the buffer
	int buffer_capacity = mem_capacity / record_size; // number of records in buffer
	int buffer_size = buffer_capacity * record_size; // number of bytes in buffer
	int record_count = 0;
	char* record_pointer = (char*) mem;	
	std::string line;

	while (infile.good()) {
		getline(infile, line); // read a line

		if (line.length() <= 0) {
		  continue; // skip over blank lines
		}

		/* if buffer is not full */
		if (record_count <= buffer_capacity) {
		  // serialize and put the record in the buffer
		  reader->serialize(line, record_pointer);

		  // update states for next iteration
		  record_pointer += record_size;
		  record_count++;
		  continue;
		} 

		/* if buffer is full */ 
		//TODO: sort the buffer

		// write it out to disk
		outfile.write((char*)mem, buffer_size);

		// reset states
		record_pointer = (char*) mem;
		memset(mem, 0, mem_capacity);		

		// serialize and put the record in the buffer
		reader->serialize(line, record_pointer);
		record_count = 1;

	} // end while

	//TODO: sort the buffer
        int (*myCompareRecords)(const void *, const void *);
        myCompareRecords = &compareRecord;

	qsort(mem, record_count, record_size, myCompareRecords);

	// write the last page to disk
	outfile.write((char*)mem, buffer_size);

	// close files
	infile.close();
	outfile.close();
}
////////////////////////////////////////////////////////////
//
//     RunIterator class   
//
////////////////////////////////////////////////////////////


RunIterator::RunIterator(std::ifstream *pagefile, long sp, long rl, long bs, SchemaReader *sr){
	fp = pagefile;
	start_pos = sp;
	run_length = rl;
	buf_size = bs;
	reader = sr;
	record_length = sr->getRecordSize();

	cur_pos = start_pos;
}

RunIterator::~RunIterator() { /* do nothing */}

Record *RunIterator::next() {
	// save current read position so as to not mess up with the other RunIterator's
	long pos = fp->tellg();

	// read next record
	char * buffer = new char[record_length];
	fp->seekg(cur_pos, fp->beg);
	fp->read(buffer, record_length);
	
	// deserialize record
	Record *record = new Record;
	reader->deserialize(buffer, record);
	delete buffer;

	// restore saved seekg position
	fp->seekg(pos);

	// increment to next record
	cur_pos += record_length;

	return record;
}
 
bool RunIterator::has_next() {
	return cur_pos < start_pos + run_length; // note: run_length has to be in bytes, not pages.	
}


////////////////////////////////////////////////////////////
//
//     mk_runs, merge_runs  
//
////////////////////////////////////////////////////////////


void mk_runs(std::string in_fn, std::string out_fn, long run_length, Schema *schema)
{
//  int record_size = calc_record_size(schema);
//  int buf_size = run_length * page_size;
//  void *buf = malloc(buf_size);
  
//  int (*myCompareRecords)(const void *, const void *);
//  myCompareRecords = &compare_records;
  
//  fread((char *) buf, buf_size, 1, in_fp);
//  qsort(buf, (page_size / record_size) * run_length , buf_size, myCompareRecords);

//  fwrite(buf, buf_size, 1, out_fp);
//  free(buf);
	ExternalSorter sorter(schema, run_length, in_fn, out_fn);
	sorter.csv2pagefile(in_fn, out_fn);
}

void merge_runs(RunIterator* iterators[], int num_runs, FILE *out_fp,
                long start_pos, char *buf, long buf_size)
{
  // Your implementation
}

int calc_record_size(Schema *schema)
{
   int length = 0, i = 0;
   Attribute * attr = schema->attrs[0];
   
   while (attr != NULL) {
      length += attr->length;
      attr = schema->attrs[++i];
   }
   return length;
}


