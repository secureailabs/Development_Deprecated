import numpy as np
import sys
import pickle
import heapq

data_heap = 0
with open(sys.argv[0], 'rb') as f:
    data_heap = pickle.load(f)

minval = heapq.heappop(data_heap)

with open(sys.argv[1], 'wb') as f:
    pickle.dump(minval, f)
with open(sys.argv[2], 'wb') as f:
    pickle.dump(data_heap, f)
