import xgboost as xgb
import numpy as np
from sklearn.metrics import confusion_matrix
from sklearn.metrics import precision_recall_curve

y = __testy.to_numpy()
#print(__testy.value_counts())
dtest = xgb.DMatrix(np.asarray(__testX), label=np.asarray(y))
test_preds = __model.predict(dtest)
test_preds = 1/(1+ np.exp(-test_preds))
#print(test_preds)

__tmp_precision, __tmp_recall, __tmp_thresholds = precision_recall_curve(y, test_preds)

probability_thresholds = __threshes

__conf_mats = []

for p in probability_thresholds:    
    test_preds_labels = []
    for prob in test_preds:
        if prob > p:
            test_preds_labels.append(1)
        else:
            test_preds_labels.append(0)
    simfl_conf = confusion_matrix(y, test_preds_labels, labels = [0,1])
    __conf_mats.append(simfl_conf)
