from scipy import stats
from scipy.stats import norm

import math
import random
import numpy
import csv

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
  i = 0
  l = []
  
  for attribute in schema:
    l.append([])
    if attribute["type"] == "string":
      for j in range(nrecords):
        l[i].append(''.join([random.choice(letters) for j in range(attribute["length"])]))
    elif attribute["type"] == "integer":
      if "distribution" in attribute:
        if attribute["distribution"]["name"] == "uniform":
          l[i].extend(numpy.random.randint(attribute["distribution"]["min"], attribute["distribution"]["max"] + 1, nrecords))
    elif attribute["type"] == "float":
      if "distribution" in attribute:
              if attribute["distribution"]["name"] == "normal":
                rv = norm(attribute["distribution"]["mu"], attribute["distribution"]["sigma"])
                a = rv.rvs(nrecords)
                l[i] = l[i] + [math.ceil(x*100)/100 for x in a] 
    i = i + 1
      
  result = zip(*l)
  
  f = open(out_file, "w")
  
  for x in result:
    out = csv.writer(f, delimiter=',', quoting=csv.QUOTE_NONE)
    out.writerow(x)

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