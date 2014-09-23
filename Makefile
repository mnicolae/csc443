# Makefile
CC = g++
CCFLAGS = -g -Wall

# all:

# write_fixed_len_pages:

read_fixed_len_pages: read_fixed_len_pages.o
	$(CC) -o $@ $(CCFLAGS) $^

# csv2heapfile:

# scan:

# insert:

# update:

# delete:

# select:

# csv2colstore:

# select2:

# select3:

%.o : src/%.cc
	$(CC) -c -o $@ $(CCFLAGS) $^ 

# clean:
