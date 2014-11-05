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

/*
 * Used by RunIterator.next()
 */
void SchemaReader::deserialize(char* data, Record *rec) {
	char* ptr = data;
	int rec_len;
	rec->schema = schema;
	rec_len = getRecordSize();
	rec->data = (char*) malloc(rec_len);
	memcpy(rec->data, data, rec_len);
}


////////////////////////////////////////////////////////////
//
//     External Sorter class   
//
////////////////////////////////////////////////////////////

ExternalSorter::ExternalSorter(SchemaReader *rdr, int mem_cap, std::string csv_fn, std::string page_fn) {
	reader = rdr;
	mem_capacity = mem_cap;
	mem = malloc(mem_capacity);
	record_size = reader->getRecordSize();
	SCHEMA = reader->getSchema();
}

ExternalSorter::ExternalSorter(std::string schema_filename) {
  reader = new SchemaReader(schema_filename);
  mem_capacity = 3072; //TODO: default value should be 3MB when running the experiments
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

SchemaReader* ExternalSorter::getSchemaReader() {
	return reader;
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

int ExternalSorter::csv2pagefile(std::string csv_file, std::string page_file) {
  std::ifstream infile(csv_file.c_str(), std::ifstream::binary);
  std::ofstream outfile(page_file.c_str(), std::ofstream::binary);

	if (!infile.is_open()) {
	  std::cout << "unable to open csv file";
	  return 0;
	}

	// initialize the buffer
	memset(mem, 0, mem_capacity);

	// calculate how many records will fit in the buffer
	int buffer_capacity = mem_capacity / record_size; // number of records in buffer
	int buffer_size = buffer_capacity * record_size; // number of bytes in buffer
	int record_count = 0;
	int total_record_count = 0;
	char* record_pointer = (char*) mem;	
	std::string line;

	// initialize pointer to compareRecord function
    int (*myCompareRecords)(const void *, const void *);
    myCompareRecords = &compareRecord;


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
		  total_record_count++;
		  record_count++;
		  continue;
		} 

		/* if buffer is full */ 
		qsort(mem, record_count, record_size, myCompareRecords);

		// write it out to disk
		outfile.write((char*)mem, buffer_size);

		// reset states
		record_pointer = (char*) mem;
		memset(mem, 0, mem_capacity);		

		// serialize and put the record in the buffer
		reader->serialize(line, record_pointer);
		record_count = 1;
		total_record_count++;

	} // end while

	// sort the buffer

	qsort(mem, record_count, record_size, myCompareRecords);

	// write the last page to disk
	outfile.write((char*)mem, buffer_size);

	// close files
	infile.close();
	outfile.close();

	return total_record_count;
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
	buffer = (char *) malloc(buf_size);
	memset(buffer, 0, buf_size);

	fillBuffer();
	cur_rec_pos = 0;

}

int RunIterator::fillBuffer() {
	if (cur_pos >= start_pos + run_length) {
		return -1; // don't want to read pass the end of the run
	}

	// save current read position so as to not mess up with the other RunIterator's
	long pos = fp->tellg();

	// load next buf_size from page file into memory
	fp->seekg(cur_pos, fp->beg);
	fp->read(buffer, buf_size);

	// restore saved seekg position
	fp->seekg(pos);

	// increment to next record
	cur_pos += buf_size;

	return 1;
}

RunIterator::~RunIterator() { free(buffer); }

Record *RunIterator::next() {

	if (cur_rec_pos >= buf_size) {
		fillBuffer();
		cur_rec_pos = 0;
	}

	// read next record
	char * rec_buf = new char[record_length];
	char* rec_ptr = (char*) buffer + cur_rec_pos;
	memcpy(rec_buf, rec_ptr, record_length);
	
	// deserialize record
	Record *record = new Record;

	reader->deserialize(rec_buf, record);
	delete rec_buf;

	// increment to next record
	cur_rec_pos += record_length;

	return record;
}
 
bool RunIterator::has_next() {

	if (cur_rec_pos >= buf_size)
	{
		if (fillBuffer() == -1)
		{
			return false;
		}
		cur_rec_pos = 0;
	}
	// check if the next record is empty (only happens in last page of run) ...
	char* rec_ptr = (char*) buffer + cur_rec_pos;
	// ... by checking if the first byte is zero
	if ((*rec_ptr == 0) && (cur_pos == start_pos + run_length))
		return false;
	return true; // note: run_length has to be in bytes, not pages.
}

SchemaReader* RunIterator::getReader() {
	return reader;
}

////////////////////////////////////////////////////////////
//
//     mk_runs, merge_runs  
//
////////////////////////////////////////////////////////////


void mk_runs(std::string in_fn, std::string out_fn, long run_length, Schema *schema)
{
//	ExternalSorter sorter(schema, run_length, in_fn, out_fn);
//	sorter.csv2pagefile(in_fn, out_fn);
}

void merge_runs(RunIterator* iterators[], int num_runs, std::ofstream *out_fp,
		long start_pos, char *buf, long buf_size) {
	int i, j;
	std::vector<char*> heap;
	int rating[num_runs];
	int compared, smaller;

	for (i = 0; i < num_runs; i++) {
		heap.push_back(iterators[i]->next()->data);
		rating[i] = i;
		for (j = i - 1; j >= 0; j--) {
			compared = compareRecord(heap[i], heap[j]);
			if (compared == 0) {
				rating[j] = rating[i];
				break;
			}

			if (compared == -1) { // i > j
				smaller = rating[j];
				rating[j] = rating[i];
				rating[i] = smaller;
			}
		}
	}

	int done = 0;
	int record_size = iterators[0]->getReader()->getRecordSize();
	int cur_rec_pos = 0; // current record position in buffer
	int buf_capacity = buf_size / record_size;
	char *buf_ptr = buf;
	while (done < num_runs) {

		if (cur_rec_pos > buf_size) {
			// if buf is full, write it out to disk
			out_fp->write(buf, buf_size);
			memset(buf, 0, buf_size);
			cur_rec_pos = 0;
		}

		// find where the smallest record, i.e. the one with rating = 0
		for (i = 0; i < num_runs && rating[i] != 0; i++) {
		}

		// put it into buf
		memcpy(buf_ptr, heap[i], record_size);
		*buf_ptr += cur_rec_pos;

		// replace smallest one with next record of the runIterator where it comes from
		if (iterators[i]->has_next()) {
			heap[i] = iterators[i]->next()->data; // TODO: this might not work
		} else {
			// if one iterator is empty, increase done by 1
			rating[i] = -1;
			done++;
		}

		// re-sort the rating
		int rating_i = 0;
		if (rating[i] == -1)
		{
			for (j = 0; j < num_runs; j++)
			{
				if (rating[j] != -1)
				{
					rating[j]--;
				}
			}
		}
		else
		{
			//TODO: fix this re-sorting
			rating[i] = 0;
			for (j = 0; j < num_runs; j++) {
				if (i == j || rating[j] == -1) {
					continue;
				}

				compared = compareRecord(heap[i], heap[j]);
				if (compared == -1) { // i < j
					rating[j]++;
				} else if (compared == 1) { // i > j
					rating_i++;
				}
			}
		}
		rating[i] = rating_i;

	} // end while


}

int main() {
	std::string in_fn = "data_example.csv";
	std::string out_fn = "pagefile";
	std::string pass1_fn = "pass1.pages";

	// create page file
	ExternalSorter sorter("schema_example.json");
	// add sort attributes to schema
	sorter.addSortingAttributes("cgpa");
	int record_count = sorter.csv2pagefile(in_fn, out_fn);



	int num_runs = 4;
	int buf_size = 125 / (num_runs + 1);
	sorter.setMemCapacity(125);
	RunIterator** iterators = new RunIterator*[num_runs];

	int record_size = 25;
	int run_length = (record_count / num_runs) * record_size;
	int last_run_length = (record_count % num_runs) * record_size;

	std::ifstream page_fp(out_fn.c_str(), std::ifstream::binary);
	int i = 0;
	for (i = 0 ; i < num_runs - 1; i++) {
		iterators[i] = new RunIterator(&page_fp, i * run_length, run_length, buf_size, sorter.getSchemaReader());
	}
	iterators[i] = new RunIterator(&page_fp, i * run_length, last_run_length, buf_size, sorter.getSchemaReader());

	std::ofstream out_fp(pass1_fn.c_str(), std::ofstream::binary);
	char * buf = (char*) malloc(buf_size);
	memset(buf, 0, buf_size);
	merge_runs(iterators, num_runs, &out_fp, 0, buf, buf_size);


}

