
# finish initialization of heap[] and rating[]
#break library.cc:571 
#command
#	print rating[0]
#	print rating[1]
#	print rating[2]
#	print rating[3]
#	print heap[0]
#	print heap[1]
#	print heap[2]
#	print heap[3]
#end	
#
#break library.cc:604 
#command
#	print heap[i]
#end

#break library.cc:640
#command
#	print summary[0]
#	print summary[1]
#	print summary[2]
#	print summary[3]
#	print heap[0]
#	print heap[1]
#	print heap[2]
#	print heap[3]
#end

#break library.cc:662
#command
#	print rating[0]
#	print rating[1]
#	print rating[2]
#	print rating[3]
#	print heap[0]
#	print heap[1]
#	print heap[2]
#	print heap[3]
#end
#
#break msort.cc:47
##break msort.cc:108

# print run_len after merge_runs
#break msort.cc:94
#command
#	print run_len
#end

# RunIterator initialization
break library.cc:452
command
	silent
	printf "Iterator %d: start_pos %d, run_length %d\n", id, start_pos, run_length
	continue
end

# break when RunIterator::has_next() returns false
#break library.cc:509
#break library.cc:517 if cur_pos >= start_pos + run_length
#command
#	printf "Iterator %d at end of run\n", id
#end

#break library.cc:517 if *rec_ptr == 0
#command
#	printf "Iterator %d reaches empty recordn\n", id
#end

break library.cc:474 if id == 0
command
	printf "Iterator %d refills buffer: cur_pos %d, start_pos %d, run_length %d\n", id, cur_pos, start_pos, run_length
	continue
end

break library.cc:501 if id == 0
command
	printf "Iterator %d next record: cur_rec_pos %d, buffer size %d\n", id, cur_rec_pos, buf_size
	continue
end


run schema_example.json 500.csv outfile 3072 4 cgpa
