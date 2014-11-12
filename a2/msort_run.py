import sys, os
from subprocess import Popen, PIPE

schema_file = 'schema_example2.json'
prereqs = [schema_file, 'data_generator.py', 'msort']

def check_prereqs(prereqs):
	def check(req):
		if (os.path.exists(req) == False):
			print req, 'is missing!'
			sys.exit()
		# else:
		# 	print req, 'ok!'
		return True
	map(check, prereqs)

def setup(rec_num, data_file):
	'''
	Create data for test
	'''
	if (os.path.exists(data_file) == False): 
		print "%d.csv is missing" % rec_num
		sys.exit()
		# Popen(['/usr/local/bin/python data_generator.py', schema_file, data_file, '%d' % rec_num]).wait()
		# cmd = 'python data_generator.py ' + schema_file + ' ' + data_file + ' ' + str(rec_num)
		# sys.wait()
		# Popen([cmd]).wait()

def teardown(data_file):
	# remove temp files
	try:
		os.remove(data_file + '.0')
	except OSError:
		pass

	try:
		os.remove(data_file + '.1')
	except OSError:
		pass

def format_output(outfile):
  def chunks(l, n):
    """ Yield successive n-sized chunks from l.
    """
    for i in xrange(0, len(l), n):
        yield l[i:i+n]

  f = open(outfile, 'r')
  l = f.readline()
  f.close();
  r = list(chunks(l,25)) # Note: record length is hard-coded

  f = open(outfile + '.formatted','w')
  for x in r:
    f.write(x + '\n')
  f.close()
	

def run_msort(rec_num, mem_cap, k, sort_attrs):
	
	## initialize variables
	data_file_root = str(rec_num)
	tmp_dir = data_file_root
	data_file = data_file_root + ".csv"
	# out_file = tmp_dir + '/' + data_file_root + '.out'
	out_file = data_file_root + '.out'

	# run set up
	setup(rec_num, data_file)

	# run msort
	msort_cmd = './msort ' + schema_file + ' ' + data_file + ' ' + out_file + ' ' + str(mem_cap) + ' ' + str(k) + ' ' + sort_attrs
	time=os.system(msort_cmd)

	# format output
	format_output(out_file)

	# run teardown
	teardown(out_file)

if __name__ == '__main__':
	check_prereqs(prereqs)
	run_msort(500, 3072, 4, 'cgpa')
	run_msort(5000, 3072, 4, 'cgpa')
	run_msort(10000, 3072, 4, 'cgpa')
	run_msort(50000, 3072, 4, 'cgpa')
	