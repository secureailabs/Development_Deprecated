import numpy as np
import sys
import pickle

size = 50
data = np.random.normal(0, 1, 50)
np.sort(data)

with open(sys.argv[0], 'wb') as f:
    pickle.dump(size, f)
with open(sys.argv[1], 'wb') as f:
    pickle.dump(data, f)
