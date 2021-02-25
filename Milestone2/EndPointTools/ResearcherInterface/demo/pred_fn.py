import xgboost as xgb
import numpy as np
import pickle
import sys

def sigmoid(x):
    return 1/(1+ np.exp(-x))

model = pickle.load(open(sys.argv[0], "rb"))
X_pred = pickle.load(open(sys.argv[1], "rb"))

dpred = xgb.DMatrix(X_pred)
y_pred = model.predict(dpred)

preds = sigmoid(y_pred)
pred_labels = np.round(preds)

pickle.dump(pred_labels, open(sys.argv[2], "wb"))
