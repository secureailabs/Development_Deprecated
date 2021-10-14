import xgboost as xgb
import numpy as np
from sklearn.metrics import confusion_matrix

y = __testy.to_numpy()
print(y)
dtest = xgb.DMatrix(np.asarray(__testX), label=np.asarray(y))
test_preds = __model.predict(dtest)
test_preds = 1/(1+ np.exp(-test_preds))
print(test_preds)

probability_thresholds = __threshes

__conf_mats = []

for p in probability_thresholds:    
    test_preds_labels = []
    for prob in test_preds:
        if prob > p:
            test_preds_labels.append(1)
        else:
            test_preds_labels.append(0)
    simfl_conf = confusion_matrix(y, test_preds_labels)
    __conf_mats.append(simfl_conf)
