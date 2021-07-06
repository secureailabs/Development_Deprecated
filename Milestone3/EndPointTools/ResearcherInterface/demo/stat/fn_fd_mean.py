import numpy as np
import pickle
import sys

col_num = 0
data = 0
with open(sys.argv[0], 'rb') as f:
    col_num = pickle.load(f)
with open(sys.argv[1], 'rb') as f:
    data = pickle.load(f)
    
col = data[:, col_num]

with open(sys.argv[2], 'wb') as f:
    pickle.dump(col.size, f)
with open(sys.argv[3], 'wb') as f:
    pickle.dump(np.mean(col), f)
