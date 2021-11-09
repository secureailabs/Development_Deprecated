import xgboost as xgb
import numpy as np
from sklearn.metrics import confusion_matrix

#input1 __model
#input2 __test_X
#input3 __test_y

#output __result

__test_X = __test_X.to_numpy()

dtest = xgb.DMatrix(np.asarray(__test_X), label=np.asarray(__test_y))
test_preds = __model.predict(dtest)
test_preds = 1/(1+ np.exp(-test_preds))
test_preds_labels = np.round(test_preds)
simfl_conf = confusion_matrix(__test_y, test_preds_labels)

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
