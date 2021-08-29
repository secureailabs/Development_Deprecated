import numpy as np
from collections import Counter

#function node has 1 input and one output
#input1: the hash function __info
#input2: X __X
#output: local hash values __hash_values_counters_local

__X = __X.to_numpy()

num_dim = np.shape(__X)[1]
r = 4.0
L = min(40, num_dim-1)
mu = 0.0
sigma = 1.0

hash_functions = []
num_dim = __info['num_dim']
r = __info['r']
L = __info['L']
mu = __info['mu']
sigma = __info['sigma']
hash_functions = __info['hash_functions']

#print("Calculating hash values")
hash_values = []
counters = []
for vector in __X:
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
__hash_values_counters_local = {}
__hash_values_counters_local['hash_values'] = hash_values
__hash_values_counters_local['counters'] = counters
print("Pickling local hash values")
