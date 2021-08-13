import xgboost as xgb
import numpy as np
from sklearn.metrics import confusion_matrix

#input1 __model
#input2 __test_X
#input3 __test_y

#output __result

print("testing start")
def sigmoid(x):
    return 1/(1+ np.exp(-x))

with open(sys.argv[0], "rb") as f:
    model = pickle.load(f)
with open(sys.argv[1], "rb") as f:
    test_X = pickle.load(f)
with open(sys.argv[2], "rb") as f:
    test_y = pickle.load(f)
with open(sys.argv[3], "rb") as f:
    df = pickle.load(f)
    
print("receiving data")

dtest = xgb.DMatrix(np.asarray(__test_X), label=np.asarray(__test_y))
test_preds = __model.predict(dtest)
test_preds = sigmoid(test_preds)
test_preds_labels = np.round(test_preds)
simfl_conf = confusion_matrix(test_y, test_preds_labels)

tn, fp, fn, tp = simfl_conf.ravel()
simfl_errors = (fn+fp)/(tn+fp+fn+tp) 
simfl_fnr = fn / (tp+fn)
simfl_fpr = fp / (tn+fp)

error_result = "error: "+str(simfl_errors) + "    false negative rate: " + str(simfl_fnr) + "    false positive rate: " + str(simfl_fpr) 
print(error_result)

__result = {}
__result["conf_mat"] = simfl_conf
__result["errors"] = simfl_errors
__result["fnr"] = simfl_fnr
__result["fpr"] = simfl_fpr
