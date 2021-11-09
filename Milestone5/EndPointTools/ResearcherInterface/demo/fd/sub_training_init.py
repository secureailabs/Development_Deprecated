import pickle
import sys
import xgboost as xgb
import numpy as np

def sigmoid(x):
    return 1/(1+ np.exp(-x))

def cust_obj(preds, training):
    labels = training.get_label()
    preds = sigmoid(preds)
    grad = preds - labels
    hess = preds*(1-preds)
    return grad, hess

#function node has 3 inputs and 1 output
# input 1: X
# input 2: y
# input 3: node info
# input 4: model
# input 5: hash table
# output: local gradients calculated

X = 0
y = 0
with open(sys.argv[0], "rb") as f:
    X = pickle.load(f)
with open(sys.argv[1], "rb") as f:
    y = pickle.load(f)

#print("Initializing node info")
node_id = 0
training_node_id = 0
num_parties = 0
initialization = 0

#print("Waiting for node ID info from parent")

with open(sys.argv[2], "rb") as f:
    info = pickle.load(f)
    training_node_id = info['training']
    node_id = info['node_id']
    num_parties = info['num_parties']
#print("Node IDs updated for current round")

#print("Starting intialization of SimFL")
#print("Initializing local dataset and copy of global model")
model = 0
with open(sys.argv[3], "rb") as f:
    model = pickle.load(f)

hash_tables = []
with open(sys.argv[4], 'rb') as f:
    hash_tables = pickle.load(f)
#print("Got similarity matrices from parent")
#print("Calculating gradients for local instances")

xgb_local = xgb.DMatrix(np.asarray(X), label=np.asarray(y))

dpred_inst = model.predict(xgb_local)
grad, hess = cust_obj(dpred_inst, xgb_local)

gradient_pairs = {}
gradient_pairs['grad'] = grad
gradient_pairs['hess'] = hess
with open(sys.argv[5], 'wb') as outfile:
    pickle.dump(gradient_pairs, outfile)
#print("Pickled local gradients")