import pickle
import sys
import numpy as np
import torch
import torch.nn as nn

gradlist = 0
with open(sys.argv[0], 'rb') as f:
    gradlist = pickle.load(f)
    
new_gradients = list(map(lambda x, y: (x + y)/2, gradlist[0], gradlist[1]))

with open(sys.argv[1], 'wb') as f:
    pickle.dump(new_gradients,f)
    f.flush()
