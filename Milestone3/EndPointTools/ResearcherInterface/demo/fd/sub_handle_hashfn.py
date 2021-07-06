import pickle
import sys
import numpy as np
from collections import Counter

#function node has 1 input and one output
#input1: the hash function
#input2: X
#output: local hash values

X=0
with open(sys.argv[0], 'rb') as f:
   X=pickle.load(f) 

#print("Obtaining hash parameters from parent")
num_dim = len(X[0])
r = 4.0
L = min(40, num_dim-1)
mu = 0.0
sigma = 1.0
hash_functions = []
with open(sys.argv[1], 'rb') as f:
    info = pickle.load(f)
    num_dim = info['num_dim']
    r = info['r']
    L = info['L']
    mu = info['mu']
    sigma = info['sigma']
    hash_functions = info['hash_functions']

#print("Calculating hash values")
hash_values = []
counters = []
for vector in X:
    hashes = []
    for hash_index in (range(L)):
        hash_val = (np.dot(hash_functions[hash_index][0], vector) + hash_functions[hash_index][1])/r
        hash_val = np.floor(hash_val)
        hash_val_float = hash_val[0]
        hashes.append(hash_val_float)
    counters.append(Counter(hashes))
    hash_values.append(hashes)
#print("Hash values calculated!")

#Sending local hash values to hub
hash_values_counters_local = {}
hash_values_counters_local['hash_values'] = hash_values
hash_values_counters_local['counters'] = counters
print("Pickling local hash values")
with open(sys.argv[2], 'wb') as outfile:
    pickle.dump(hash_values_counters_local, outfile)