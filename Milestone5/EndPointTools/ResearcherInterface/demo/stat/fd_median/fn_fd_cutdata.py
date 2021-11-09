import sys
import pickle
import numpy as np

data = 0
num = 0
dbit = 0

with open(sys.argv[0], 'rb') as f:
    data = pickle.load(f)
with open(sys.argv[1], 'rb') as f:
    num = pickle.load(f)
with open(sys.argv[2], 'rb') as f:
    dbit = pickle.load(f)

if(dbit == 0):
    data = data[num:]
elif(dbit == 1):
    data = data[:num]

with open(sys.argv[3], 'rb') as f:
    pickle.dump(data, f)
