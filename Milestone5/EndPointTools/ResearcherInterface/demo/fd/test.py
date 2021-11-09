import pickle
import sys
import xgboost as xgb
import numpy as np
from sklearn.metrics import confusion_matrix
import shap
import matplotlib.pyplot as pl

def sigmoid(x):
    return 1/(1+ np.exp(-x))

with open(sys.argv[0], "rb") as f:
    test_X = pickle.load(f)
with open(sys.argv[1], "rb") as f:
    test_y = pickle.load(f)
with open(sys.argv[2], "rb") as f:
    df = pickle.load(f)
with open(sys.argv[3], "rb") as f:
    model = pickle.load(f)

dtest = xgb.DMatrix(np.asarray(test_X), label=np.asarray(test_y))
test_preds = model.predict(dtest)
test_preds = sigmoid(test_preds)
test_preds_labels = np.round(test_preds)
simfl_conf = confusion_matrix(test_y, test_preds_labels)

tn, fp, fn, tp = simfl_conf.ravel()
simfl_errors = (fn+fp)/(tn+fp+fn+tp) 
simfl_fnr = fn / (tp+fn)
simfl_fpr = fp / (tn+fp)

error_result = "error: "+str(simfl_errors) + "\nfalse negative rate: " + str(simfl_fnr) + "\nfalse positive rate: " + str(simfl_fpr) 

shap.initjs()
explainer = shap.TreeExplainer(model)
shap_values = explainer.shap_values(df)
shap.summary_plot(shap_values, df, show=False)

with open(sys.argv[4], "wb") as f:
    pickle.dump(simfl_conf, f)
with open(sys.argv[5], "wb") as f:
    pickle.dump(error_result, f)
with open(sys.argv[6], "wb") as f:
    pickle.dump(pl.gcf(), f)