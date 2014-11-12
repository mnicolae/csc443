
#include "library.h"


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

	// initialize temporary files
	int cur_idx = 0;
	std::string tmp_file_name[] = {"0.tmp", "1.tmp"};
 	std::fstream *tmp_file = new std::fstream[2];

  	

	ftime(&start);
	start_time = start.time * 1000 + start.millitm;


	// read schema
	SchemaReader reader(schema_fn);
	ExternalSorter sorter(&reader, mem_capacity);
	sorter.addSortingAttributes(sortingAttrs);

	// initialize the run_length vector
	std::vector<long> run_lengths[2];

	// make runs
	sorter.csv2pagefile(csv_fn, tmp_file_name[cur_idx], &run_lengths[cur_idx]);

	// DEBUG
	for(std::vector<long>::const_iterator i=run_lengths[cur_idx].begin(); i !=run_lengths[cur_idx].end(); i++)
		std::cout << *i << ", " ; 

	////////////////////////////////////////////////////////////
	//
	//    Main merge loop 
	//
	////////////////////////////////////////////////////////////
	int record_size = reader.getRecordSize();
	long tmp_size = mem_capacity / (k+1);
	long buffer_size = (tmp_size / record_size) * record_size;
	long start_pos;
	long run_len;
	RunIterator** iterators;
	
	while (run_lengths[cur_idx].size() > 1) { // Note: don't change this to total_num_runs

		// open the temp files: open cur_idx in read mode and (1 - cur_idx) in write mode
		tmp_file[cur_idx].open(tmp_file_name[cur_idx].c_str(), std::fstream::in | std::fstream::binary);
		tmp_file[1 - cur_idx].open(tmp_file_name[1 - cur_idx].c_str(), std::fstream::out | std::fstream::binary);		

		// initialize vector that stores run lengths for next pass	
		run_lengths[1 - cur_idx].clear();


		// call merge_runs multiple times to merge all runs in this pass
		int num_runs;
		int total_num_runs = run_lengths[cur_idx].size();
		for (int i=0; i < total_num_runs; i += k) { 

		  	// initialize a group of _at most_ k iterators
			iterators = new RunIterator*[k];
			start_pos = 0;
			num_runs = 0;
			for (int j = 0; j < k; j++) {
				if (i + j < total_num_runs ) {
					iterators[j] = new RunIterator(j, &tmp_file[cur_idx], start_pos, run_lengths[cur_idx][i + j], buffer_size, &reader);
					start_pos += run_lengths[cur_idx][i + j];
					num_runs ++;
				}
			}

		  	// merge the k iterators
			char * buffer = (char*) malloc(buffer_size);
			memset(buffer, 0, buffer_size);
			run_len = merge_runs(iterators, num_runs, &tmp_file[1 - cur_idx], 0, buffer, buffer_size);

		  	//push run_len into the next run_lengths array
			run_lengths[1 - cur_idx].push_back(run_len);

		  	// destroy this group of iterators
			for (int j = 0; j < num_runs; j++) {
				delete iterators[j];
			}

			// delete the buffer
			free(buffer);
		}

	  	// close the files
		tmp_file[cur_idx].close();
		tmp_file[1 - cur_idx].close();

	 	// switch the index
		std::cout<< tmp_file_name[1-cur_idx] << "\n";

		// DEBUG
		for(std::vector<long>::const_iterator i=run_lengths[1-cur_idx].begin(); i !=run_lengths[1- cur_idx].end(); i++)
			std::cout << *i << ", " ; 

		cur_idx = 1 - cur_idx;
	}

	////////////////////////////////////////////////////////////
	//
	//    End of main merge loop 
	//
	////////////////////////////////////////////////////////////

	rename( tmp_file_name[cur_idx].c_str() ,  out_fn);

	ftime(&end);
	end_time = end.time * 1000 + end.millitm;  

	printf("\nTIME: %ld miliseconds\n", end_time - start_time);

	return 0;
}

