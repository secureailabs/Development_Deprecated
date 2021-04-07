import numpy as np
import sys
import pickle

data = 0
with open(sys.argv[0], 'rb') as f:
    data = pickle.load(f)

med = 0
med_idx = data.size/2
if(data.size%2!=0):
    med = data[med_idx]
elif(data.size%2==0):
    med = (data[med_idx]+data[med_idx-1])/2

with open(sys.argv[1], 'wb') as f:
    pickle.dump(data.size, f)
with open(sys.argv[2], 'wb') as f:
    pickle.dump(med, f)

