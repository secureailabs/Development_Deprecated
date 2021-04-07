import numpy as np
from sklearn.metrics import mean_absolute_error
import torch
import sys
import pickle

y_pred = 0
y_true = 0
with open(sys.argv[0], 'rb') as f:
    y_pred = pickle.load(f)
with open(sys.argv[1], 'rb') as f:
    y_true = pickle.load(f)
    
err = mean_absolute_error(y_true, y_pred)

with open(sys.argv[2], 'wb') as f:
    pickle.dump(err, f)
