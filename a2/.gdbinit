# write in csv2pagefile
break library.cc:398 
break library.cc:420

#break fillBuffer

# initialize runiterators
break msort.cc:80 
command
	print j
	continue
end

# finish initialization of heap[] and rating[]
break library.cc:571 
command
	print rating[0]
	print rating[1]
	print rating[2]
	print rating[3]
end	

# inside RunIterator constructor, after fillBuffer is called
#break library.cc:453

# inside fillBuffer, after fp->read is called
break library.cc:470
command
	print buffer
end

run schema_example.json 500.csv outfile 3072 4 cgpa
