#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "leveldb/db.h"
#include "json/json.h"

using namespace std;

int main(int argc, const char* argv[]) {

	if (argc != 4) {
		cout << "ERROR: invalid input parameters!" << endl;
		cout << "Please enter <schema_file> <input_file> <out_index>" << endl;
		exit(1);
	}

	// Do work here
	
	return 0;
}