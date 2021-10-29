import xgboost as xgb
from sklearn.metrics import precision_recall_curve

model_params = {
"colsample_bylevel": 0.8523804169779899,
"colsample_bytree": 0.4004491523598215,
"gamma": 1e-09,
"learning_rate": 0.07585296847082715,
"max_delta_step": 20,
"max_depth": 9,
"n_estimators": 150,
"reg_alpha": 6.8398027190415516e-06,
"reg_lambda": 100.0,
"subsample": 1.0,
"scale_pos_weight": 1.0
}

estimator = xgb.XGBClassifier(**model_params, objective = 'binary:logistic', n_jobs=-1, eval_metric='aucpr',
                              use_label_encoder=False)
model = estimator.fit(__X, __y)

preds = estimator.predict(__X)

__precision, __recall, __thresholds = precision_recall_curve(__y, preds)
