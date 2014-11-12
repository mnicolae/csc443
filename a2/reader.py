import sys

def chunks(l, n):
    """ Yield successive n-sized chunks from l.
    """
    for i in xrange(0, len(l), n):
        yield l[i:i+n]

f = open(sys.argv[1], 'r')
l = f.readline()
f.close();
r = list(chunks(l,25))

f = open(sys.argv[1] + '.sorted','w')
for x in r:
	f.write(x + '\n')
f.close()
