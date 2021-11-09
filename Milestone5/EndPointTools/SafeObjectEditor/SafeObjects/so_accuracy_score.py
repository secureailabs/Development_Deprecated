from sklearn.metrics import accuracy_score
import xgboost as xgb
import numpy as np

y = __testy.to_numpy()
dtest = xgb.DMatrix(np.asarray(__testX), label=np.asarray(y))
test_preds = __model.predict(dtest)
test_preds = 1/(1+ np.exp(-test_preds))
test_preds_labels = np.round(test_preds)
__score = accuracy_score(y, test_preds_labels)
