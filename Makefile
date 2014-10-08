# Makefile
CC = g++
CCFLAGS = -g -Wall
INCLUDES = -I. -I./src/*.h

TARGETS: write_fixed_len_pages read_fixed_len_pages csv2heapfile scan insert update delete select
OBJSDEPS: 
all: $(TARGETS)

write_fixed_len_pages: write_fixed_len_pages.o pageManager.o bufferManager.o 
	$(CC) -o $@ $(CCFLAGS) $^         

read_fixed_len_pages: read_fixed_len_pages.o pageManager.o bufferManager.o 
	$(CC) -o $@ $(CCFLAGS) $^

csv2heapfile: csv2heapfile.o pageManager.o bufferManager.o heapManager.o 
	$(CC) -o $@ $(CCFLAGS) $^         

scan: scan.o pageManager.o bufferManager.o heapManager.o
	$(CC) -o $@ $(CCFLAGS) $^         

insert: insert.o pageManager.o bufferManager.o heapManager.o
	$(CC) -o $@ $(CCFLAGS) $^         

update: update.o pageManager.o bufferManager.o heapManager.o
	$(CC) -o $@ $(CCFLAGS) $^         

delete: delete.o pageManager.o bufferManager.o heapManager.o
	$(CC) -o $@ $(CCFLAGS) $^         

select: select.o pageManager.o bufferManager.o heapManager.o
	$(CC) -o $@ $(CCFLAGS) $^         

csv2colstore: csv2colstore.o pageManager.o bufferManager.o heapManager.o

# select2:

# select3:

%.o : src/%.cc
	$(CC) -c -o $@ $(CCFLAGS) $(INCLUDES) $^ 

clean:
	rm -f *.o
