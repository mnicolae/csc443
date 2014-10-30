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

void SchemaReader::deserialize(char* data, Record *rec) {

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

RunIterator::~RunIterator() { /* do nothing */}

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

	// check if the next record is empty (only happens in last page of run) ...
	char* rec_ptr = (char*) buffer + cur_rec_pos;
	// ... by checking if the first byte is zero
	if ((*rec_ptr == 0) && (cur_pos == start_pos + run_length))
		return false;
	return true; // note: run_length has to be in bytes, not pages.
}

int RunIterator::get_number_of_records() {
	int cur_pos_tmp = cur_pos;
        int count = 0;

        while (cur_pos_tmp < start_pos + run_length) {
		count += 1;
		cur_pos_tmp += record_length;
	}

	return count;        
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


////////////////////////////////////////////////////////////
//
//     Min Heap functions
//
////////////////////////////////////////////////////////////



// TODO finish this part
// http://www.geeksforgeeks.org/merge-k-sorted-arrays/

// A min heap node
struct MinHeapNode
{
    Record element; // The element to be stored
    int i; // index of the array from which the element is taken
    int j; // index of the next element to be picked from array
};

// Prototype of a utility function to swap two min heap nodes
void swap(MinHeapNode *x, MinHeapNode *y);

// A class for Min Heap
class MinHeap
{
    MinHeapNode *harr; // pointer to array of elements in heap
    int heap_size; // size of min heap
public:
    // Constructor: creates a min heap of given size
    MinHeap(MinHeapNode a[], int size);
 
    // to heapify a subtree with root at given index
    void MinHeapify(int );
 
    // to get index of left child of node at index i
    int left(int i) { return (2*i + 1); }
 
    // to get index of right child of node at index i
    int right(int i) { return (2*i + 2); }
 
    // to get the root
    MinHeapNode getMin() { return harr[0]; }
 
    // to replace root with new node x and heapify() new root
    void replaceMin(MinHeapNode x) { harr[0] = x;  MinHeapify(0); }
};



// FOLLOWING ARE IMPLEMENTATIONS OF STANDARD MIN HEAP METHODS
// FROM CORMEN BOOK
// Constructor: Builds a heap from a given array a[] of given size
MinHeap::MinHeap(MinHeapNode a[], int size)
{
    heap_size = size;
    harr = a;  // store address of array
    int i = (heap_size - 1)/2;
    while (i >= 0)
    {
        MinHeapify(i);
        i--;
    }
}

// A recursive method to heapify a subtree with root at given index
// This method assumes that the subtrees are already heapified
void MinHeap::MinHeapify(int i)
{
    int l = left(i);
    int r = right(i);
    int smallest = i;

    if (l < heap_size && compareRecord(harr[l].element.data, harr[i].element.data) == -1)
        smallest = l;
    if (r < heap_size && compareRecord(harr[r].element.data, harr[smallest].element.data) == -1)
        smallest = r;
    if (smallest != i)
    {
        swap(&harr[i], &harr[smallest]);
        MinHeapify(smallest);
    }
}

// A utility function to swap two elements
void swap(MinHeapNode *x, MinHeapNode *y)
{
    MinHeapNode temp = *x;  *x = *y;  *y = temp;
}


void merge_runs(RunIterator* iterators[], int num_runs, FILE *out_fp,
                long start_pos, char *buf, long buf_size)
{
  // Your implementation

  // TODO finish



}


int main() {
	SchemaReader reader("schema_example.json");
	int run_length = 2;
	std::string in_fn = "";
	std::string out_fn = "";
	ExternalSorter sorter(reader.getSchema(), run_length, in_fn, out_fn); //TODO define the constructor
	sorter.csv2pagefile(in_fn, out_fn);


	int num_runs = 4;

	MinHeapNode *harr = new MinHeapNode[num_runs];
	RunIterator* iterators = new RunIterator[num_runs];
	  for (int i = 0; i < num_runs; i++) {
	     harr[i].element = iterators[i]->next();
	     harr[i].i = i;
	     harr[i].j = 1;
	  }
	  MinHeap hp(harr, num_runs);

	  // Now one by one get the minimum element from min
	  // heap and replace it with next element of its array
	  int empty_runs = 0;
	  int count = 0;
	  while (1) {

		  for(int j = 0; j < num_runs; j++) {
			  if (!iterators[i]->has_next())
				  empty_runs++;
		  }

		  if (empty_runs < num_runs) {
			  empty_runs = 0;
		  } else if (empty_runs == num_runs) { // no more records
			  break;
		  }

	      // Get the minimum element and store it in output
	      MinHeapNode root = hp.getMin();
	      output[count] = root.element;

	      // Find the next element that will replace current
	      // root of heap. The next element belongs to same
	      // array as the current root.
	      if (iterator[root.i]->has_next())
	      {
	          root.element = arr[root.i][root.j];
	          root.j += 1;
	      }
	      // If root was the last element of its array
	      else  root.element = BIGGEST_RECORD; //TODO: define BIGGEST_RECORD

	      // Replace root with next element of array
	      hp.replaceMin(root);
	  }
}

