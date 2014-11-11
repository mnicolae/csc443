def chunks(l, n):
    """ Yield successive n-sized chunks from l.
    """
    for i in xrange(0, len(l), n):
        yield l[i:i+n]

f = open("1.tmp")
l = f.readline()
f.close();
r = list(chunks(l,25))

f = open("sorted.tmp")
for x in r:
	f.write(x + '\n')
f.close()
