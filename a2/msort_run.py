import sys, os, re
from subprocess import Popen, PIPE
import matplotlib
from pylab import *

schema_file = 'schema_example2.json'
MB = 1024 * 1024
GB = 1024 * MB

def check_prereqs(prereqs):
	def check(req):
		if (os.path.exists(req) == False):
			print req, 'is missing! Exit now ... '
			sys.exit()
		return True
	map(check, prereqs)

def setup(rec_num, data_file):
	'''
	Create data for test
	'''
	if (os.path.exists(data_file) == False): 
		print "Creating %d.csv ..." % rec_num
		Popen(['python', os.curdir +'/'+ 'data_generator.py', schema_file, data_file, str(rec_num)], stdout=PIPE).wait()

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
	s = Popen(['./msort', schema_file, data_file, out_file, str(mem_cap), str(k), sort_attrs], stdout=PIPE).stdout.read()
	r = map(int, re.findall(r'\d+', s))

	# format output
	format_output(out_file)

	# run teardown
	teardown(out_file)

	return r

def test_msort(rec_num, mem_cap, k, sort_attrs, reps):
	'''
	Execute run_msort reps times and return the average run time
	'''
	print "Test: %d-way sorting %d records on attrs '%s' with memory capacity %d ... " % (k, rec_num, sort_attrs, mem_cap)
	perf = []
	for x in range(reps):
		r = run_msort(rec_num, mem_cap, k, sort_attrs)
		perf.append(r[-1])
	return reduce(lambda x,y: x+y, perf)/len(perf)

def create_graph(tit, x, y, xlab, ylab):
	figure(1)
	title(tit)
	loglog(x,y)
	xlabel(xlab)
	ylabel(ylab)
	savefig(tit + '.png')

def do_tests(mem_caps, kways, file_sizes):
	for mem in mem_caps:
		for k in kways:
			times = []
			for size in file_sizes:
				times.append(test_msort(size, mem, k, 'cgpa', 3))
			tit =	'msort_k' + str(k) + '_m' + str(mem)
			create_graph(tit, file_sizes, times, 'file size', 'milliseconds')

if __name__ == '__main__':

	# check if all needed files exist
	prereqs = [schema_file, 'data_generator.py', 'msort']
	check_prereqs(prereqs)

	# get input
	arg_names = ['command', 'rec_num', 'mem_cap', 'k', 'sort_attrs']
	args = dict(zip(arg_names, sys.argv))


	if ('rec_num' in args) and (args['rec_num'] == 'test'):
		
		##########################################
		##
		## Change these values before running tests
		##
		##########################################

		mem_caps = [3072]
		kways = [4]
		file_sizes = [10*MB, 20*MB, 50*GB]

		##########################################
		# run tests
		do_tests(mem_caps, kways, file_sizes)
	else:
		
		# set default values
		rec_num = 5000 if ('rec_num' not in args) else int(args['rec_num'])
		mem_cap = 3072 if ('mem_cap' not in args) else int(args['mem_cap'])
		k = 4 if ('k' not in args) else int(args['k'])
		sort_attrs = 'cgpa' if ('sort_attrs' not in args) else args['sort_attrs']

		print "msort: records=%d, memory=%d, k=%d, sort_attrs='%s'" % (rec_num, mem_cap, k, sort_attrs)
		stdout = run_msort(rec_num, mem_cap, k, sort_attrs)
		print stdout