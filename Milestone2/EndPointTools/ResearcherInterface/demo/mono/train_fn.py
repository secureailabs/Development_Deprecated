import xgboost as xgb
import numpy as np
import pickle
import sys

def sigmoid(x):
    return 1/(1+ np.exp(-x))

# Log loss for logistic regression
def cust_obj(preds, training):
    labels = training.get_label()
    preds = sigmoid(preds)
    grad = preds - labels
    hess = preds*(1-preds)
    return grad, hess

holdout_X = pickle.load(open(sys.argv[0], "rb"))
holdout_y = pickle.load(open(sys.argv[1], "rb"))
params = pickle.load(open(sys.argv[2], "rb"))

dtrainAll = xgb.DMatrix(np.asarray(holdout_X[:int(len(holdout_X))]), label=np.asarray(holdout_y[:int(len(holdout_y))]))

modelAll = xgb.Booster(params, [dtrainAll])
for i in range(20):
    predictions = modelAll.predict(dtrainAll)
    g,h = cust_obj(predictions, dtrainAll)
    modelAll.boost(dtrainAll,g,h)
    
pickle.dump(modelAll, open(sys.argv[3], "wb"))
