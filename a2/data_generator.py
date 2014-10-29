from scipy import stats
from scipy.stats import norm

import math
import random
import numpy
import csv
import os

def write2file(filename, records):
  f = open(filename, "w")
  for r in records:
    f.write(str(r) + '\n')
  f.close()


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
  letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  numbers = "0123456789"
  
  print "Generating %d records" % nrecords
  
  for attribute in schema:
    if attribute["type"] == "string":
      tmp = []
      for k in range(nrecords):
        tmp.append(''.join([random.choice(letters) for j in range(attribute["length"])]))
      write2file(attribute["name"] + ".tmp", tmp)
    elif attribute["type"] == "integer":
      if "distribution" in attribute:
        if attribute["distribution"]["name"] == "uniform":
          tmp = []
          tmp.extend(numpy.random.randint(attribute["distribution"]["min"], attribute["distribution"]["max"] + 1, nrecords))
          write2file(attribute["name"] + ".tmp", tmp)
    elif attribute["type"] == "float":
      if "distribution" in attribute:
        if attribute["distribution"]["name"] == "normal":
          rv = norm(attribute["distribution"]["mu"], attribute["distribution"]["sigma"])
          a = rv.rvs(nrecords)
          write2file(attribute["name"] + ".tmp", [math.ceil(x*100)/100 for x in a])

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
  import sys, json
  if not len(sys.argv) == 4:
    print "data_generator.py <schema json file> <output csv file> <# of records>"
    sys.exit(2)

  schema = json.load(open(sys.argv[1]))
  output = sys.argv[2]
  nrecords = int(sys.argv[3])
  
  print schema
  
  generate_data(schema, output, nrecords)
