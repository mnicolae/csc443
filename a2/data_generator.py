from scipy import stats
from scipy.stats import norm
from multiprocessing import Pool

import math
import random
import numpy
import csv
import os
import sys, json

def write2file(filename, records):
  f = open(filename, "w")
  for r in records:
    f.write(str(r) + '\n')
  f.close()

'''
Generate string with length len
'''
letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
numbers = "0123456789"
def generate_strings(nrecords, len):
    tmp = []
    for k in range(nrecords):
        tmp.append(''.join([random.choice("ABCDEFGHIJKLMNOPQRSTUVWXYZ") for j in range(len)]))
    return tmp

'''
Generate uniformly distributed integers
'''
def generate_ints(nrecords, min, max):
    tmp = []
    tmp.extend(numpy.random.randint(min, max + 1, nrecords))
    return tmp

'''
Generate float using normal distribution
'''
def generate_floats(nrecords, mu, sigma):
    rv = norm(mu, sigma)
    a = rv.rvs(nrecords)
    tmp = [math.ceil(x*100)/100 for x in a]
    return tmp

'''
generate data and create a file
'''
def create_attr_file(data):
    attribute = data[0]
    nrecords = data[1]
    if attribute["type"] == "string":
        tmp = generate_strings(nrecords, attribute["length"])
        write2file(attribute["name"] + ".tmp", tmp)
    elif attribute["type"] == "integer":
        if "distribution" in attribute:
            if attribute["distribution"]["name"] == "uniform":
                tmp = generate_ints(nrecords, attribute["distribution"]["min"], attribute["distribution"]["max"])
                write2file(attribute["name"] + ".tmp", tmp)
    elif attribute["type"] == "float":
        if "distribution" in attribute:
            if attribute["distribution"]["name"] == "normal":
                tmp = generate_floats(nrecords, attribute["distribution"]["mu"], attribute["distribution"]["sigma"])
                write2file(attribute["name"] + ".tmp", tmp)

'''
You should implement this script to generate test data for your
merge sort program.

The schema definition should be separate from the data generation
code. See example schema file `schema_example.json`.
'''

def generate_data(schema, out_file, nrecords):
  '''
  Generate data according to the `schema` given,
  and write to `out_file`.
  `schema` is an list of dictionaries in the form of:
    [ 
      {
        'name' : <attribute_name>, 
        'length' : <fixed_length>,
        ...
      },
      ...
    ]
  `out_file` is the name of the output file.
  The output file must be in csv format, with a new line
  character at the end of every record.
  '''
  
  print "Generating %d records ... " % nrecords
  
  # create a worker process for each attribute
  pool = Pool()
  args = zip(schema, [nrecords for i in range(len(schema))])
  pool.map(create_attr_file, args)
  pool.close()
  pool.join()

  # open the temporary column files
  files = []
  for attribute in schema:
    files.append(open(attribute["name"] + ".tmp"))

  # re-assemble row from columns
  outfile = open(out_file, "w")
  out = csv.writer(outfile, delimiter=',', quoting=csv.QUOTE_NONE)
  for k in range(nrecords):
    line = []
    for f in files:
      line.append(f.readline().strip())
    out.writerow(line)

  # close files
  for f in files:
    f.close()
    os.remove(f.name);
  outfile.close();


if __name__ == '__main__':
  if not len(sys.argv) == 4:
    print "data_generator.py <schema json file> <output csv file> <# of records>"
    sys.exit(2)

  schema = json.load(open(sys.argv[1]))
  output = sys.argv[2]
  nrecords = int(sys.argv[3])
  
  print schema
  
  generate_data(schema, output, nrecords)
