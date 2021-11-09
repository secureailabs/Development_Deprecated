import numpy as np
import sys
import pickle

data = 0
col_num = 0

with open(sys.argv[0], 'rb') as f:
    col_num = pickle.load(f)
with open(sys.argv[1], 'rb') as f:
    data = pickle.load(f)

col = data[:, col_num]

with open(sys.argv[2], 'wb') as f:
    pickle.dump(np.amax(col), f)
