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
# input 1: X __X
# input 2: y __y
# input 3: node info __info
# input 4: model __model
# output: local gradients calculated __gradient_pairs

#print("Initializing node info")
node_id = 0
training_node_id = 0
num_parties = 0
initialization = 0

#print("Waiting for node ID info from parent")

training_node_id = __info['training']
node_id = __info['node_id']
num_parties = __info['num_parties']
#print("Node IDs updated for current round")

#print("Starting intialization of SimFL")
#print("Initializing local dataset and copy of global model")

#print("Got similarity matrices from parent")
#print("Calculating gradients for local instances")

xgb_local = xgb.DMatrix(np.asarray(__X), label=np.asarray(__y))

dpred_inst = __model.predict(xgb_local)
grad, hess = cust_obj(dpred_inst, xgb_local)

__gradient_pairs = {}
__gradient_pairs['grad'] = grad
__gradient_pairs['hess'] = hess
#print("Pickled local gradients")
