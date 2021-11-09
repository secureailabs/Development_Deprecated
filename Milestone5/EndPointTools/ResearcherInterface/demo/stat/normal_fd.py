import sys
import pickle
import numpy as np 

data = 0
xmin = 0
xmax = 0
col_num = 0

with open(sys.argv[0], 'rb') as f:
    col_num = pickle.load(f)
with open(sys.argv[1], 'rb') as f:
    xmin = pickle.load(f)
with open(sys.argv[2], 'rb') as f:
    xmax = pickle.load(f)
with open(sys.argv[3], 'rb') as f:
    data = pickle.load(f)

col = data[:, col_num]
for x in col:
    x = (x-xmin)/(xmax-xmin)

with open(sys.argv[4], 'wb') as f:
    pickle.dump(col, f)
