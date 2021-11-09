import sys
import pickle
import numpy as np 

data = 0
col_num = 0

with open(sys.argv[0], 'rb') as f:
    col_num = pickle.load(f)
with open(sys.argv[1], 'rb') as f:
    data = pickle.load(f)
    
col = data[:, col_num]
col = np.log(col)

with open(sys.argv[2], 'wb') as f:
    pickle.dump(col, f)
