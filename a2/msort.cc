#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

#include "library.h"
#include "json/json.h"

#include <string.h>
#include <time.h>
#include <math.h>
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

  char* schema_fn = (char*) argv[1];
  char* csv_fn = (char*) argv[2];
  char* out_fn =(char*) argv[3];
  int mem_capacity = atoi(argv[4]);
  int k = atoi(argv[5]);
  char* sortingAttrs = (char*) argv[6];


  SchemaReader reader(schema_fn);
  ExternalSorter sorter(&reader, mem_capacity);
  sorter.addSortingAttributes(sortingAttrs);

  // open temp files to write intermediate results
  std::fstream *tmp_file = new std::fstream[2];
  for (int i=0; i < 2; i++) {
          char fn[5];
          sprintf(fn, "%d.tmp", i);
          tmp_file[i].open(fn, std::fstream::out | std::fstream::in | std::fstream::binary);
  }

  ftime(&start);
  start_time = start.time * 1000 + start.millitm;

  // make runs
  int cur_idx = 0;
  int record_count = sorter.csv2pagefile(csv_fn, &tmp_file[cur_idx]);

  // Do the merge
  int record_size = reader.getRecordSize();
  int groups = ceil((record_count*record_size) / (mem_capacity * k));

  while (run_lengths[cur_idx].size() >= k) {
	  // initialize the run_length vector
	  std::vector<long> run_lengths[2];
	  int run_len = mem_capacity;
	  int residual = record_size * record_count;

	  RunIterator** iterators;
	  for (int i = 0; i < groups; i++) {
		  for (int j = 0; j < k; j++) {
			  if (residual > run_len) {
				  run_lengths[cur_idx].push_back(run_len);
				  residual -= run_len;
			  } else {
				  run_lengths[cur_idx].push_back(residual);
			  }
		  }
	  }

	  long buffer_size = mem_capacity / (k+1);
	  long start_pos;

	  run_lengths[1 - cur_idx].clear();
	  for (int i=0; i < groups; i++) {

		  // initialize a group of k iterators
		  iterators = new RunIterator*[k];
		  start_pos = 0;
		  for (int j = 0; j < k; j++) {
			  iterators[j] = new RunIterator(&tmp_file[cur_idx], start_pos, run_lengths[cur_idx][i*k + j], buffer_size, &reader);
			  start_pos += run_lengths[cur_idx][i*k + j];
		  }

		  // merge the k iterators
		  char * buf = (char*) malloc(buffer_size);
		  memset(buf, 0, buffer_size);
		  run_len = merge_runs(iterators, k, &tmp_file[1 - cur_idx], 0, buf, buffer_size);

		  //push run_len into the next run_lengths array
		  run_lengths[1 - cur_idx].push_back(run_len);

		  // destroy this group of iterators
		  for (int j = 0; j < k; j++) {
			  delete iterators[j];
		  }
	  }

	  // close the files

	  // switch the index
	  cur_idx = 1 - cur_idx;
  }



  ftime(&end);
  end_time = end.time * 1000 + end.millitm;  

  printf("\nTIME: %ld miliseconds\n", end_time - start_time);

  return 0;
}

