
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

break msort.cc:94

run schema_example.json 500.csv outfile 3072 4 cgpa
