import xgboost as xgb
import numpy as np

#def cust_obj(preds, training):
#    labels = training.get_label()
#    preds = 1/(1+ np.exp(-preds))
#    grad = preds - labels
#    hess = preds*(1-preds)
#    return grad, hess

#function node has 3 inputs and 1 output
# input 1: node info __info
# input 2: model __model
# input 3: X __X
# input 4: y __y
# output: local gradients calculated __gradient_pairs

__X = __X.to_numpy()

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

labels = xgb_local.get_label()
preds = 1/(1+ np.exp(-dpred_inst))
grad = preds - labels
hess = preds*(1-preds)

#grad, hess = cust_obj(dpred_inst, xgb_local)

__gradient_pairs = {}
__gradient_pairs['grad'] = grad
__gradient_pairs['hess'] = hess
#print("Pickled local gradients")
