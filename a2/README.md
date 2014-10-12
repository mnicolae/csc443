mergesort
=========

Starter code for merge sort assignment

First you need to download and compile the [leveldb](https://github.com/google/leveldb) library:

	wget https://leveldb.googlecode.com/files/leveldb-1.15.0.tar.gz
	tar -zvxf leveldb-1.15.0.tar.gz
	cd leveldb-1.15.0
	make
	cd ..

Then you need to download and compile the [jsoncpp](https://github.com/open-source-parsers/jsoncpp) in order to read JSON file.

	# Go to another directory
	cd ..
	git clone https://github.com/open-source-parsers/jsoncpp.git
	cd jsoncpp
	# Generate amalgamated source and header
	python amalgamate.py

Now copy the generated source and headers to the mergesort directory:

	cd ../mergesort
	cp ../jsoncpp/dist/jsoncpp.cpp .
	cp -r ../jsoncpp/dist/json .

Then you can compile the starter code:

	make all

