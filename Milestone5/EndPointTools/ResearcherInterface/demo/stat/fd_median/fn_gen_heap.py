import numpy as np
import sys
import pickle
import heapq

data = 0
with open(sys.argv[0], 'rb') as f:
    data = pickle.load(f)

data_heap = data.tolist()
heapq.heapify(data_heap)

with open(sys.argv[1], 'wb') as f:
    pickle.dump(data_heap, f)
