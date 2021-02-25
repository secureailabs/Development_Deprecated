import xgboost as xgb
import numpy as np
import pickle
import sys
from sklearn.metrics import confusion_matrix
import shap
import matplotlib.pyplot as pl

def sigmoid(x):
    return 1/(1+ np.exp(-x))

model = pickle.load(open(sys.argv[0], "rb"))
X_test = pickle.load(open(sys.argv[1], "rb"))
y_test = pickle.load(open(sys.argv[2], "rb"))
df = pickle.load(open(sys.argv[3], "rb"))

dtest = xgb.DMatrix(np.asarray(X_test[:int(len(X_test))]), label=np.asarray(y_test[:int(len(y_test))]))
test_preds = model.predict(dtest)
test_preds = sigmoid(test_preds)
test_preds_labels = np.round(test_preds)
simfl_conf = confusion_matrix(y_test[:int(len(y_test))], test_preds_labels)

tn, fp, fn, tp = simfl_conf.ravel()
simfl_errors = (fn+fp)/(tn+fp+fn+tp) 
simfl_fnr = fn / (tp+fn)
simfl_fpr = fp / (tn+fp)

shap.initjs()
explainer = shap.TreeExplainer(model)
shap_values = explainer.shap_values(df)
shap.summary_plot(shap_values, df, show=False)

pickle.dump(simfl_conf, open(sys.argv[4], "wb"))
pickle.dump(simfl_errors, open(sys.argv[5], "wb"))
pickle.dump(simfl_fnr, open(sys.argv[6], "wb"))
pickle.dump(simfl_fpr, open(sys.argv[7], "wb"))
pickle.dump(pl.gcf(), open(sys.argv[8], "wb"))
