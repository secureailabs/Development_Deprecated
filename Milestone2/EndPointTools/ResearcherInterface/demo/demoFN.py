import numpy as np
from sklearn.model_selection import train_test_split
import xgboost as xgb
import pandas as pd
from sklearn.preprocessing import LabelEncoder
from sklearn.metrics import precision_score, recall_score, accuracy_score, confusion_matrix
import matplotlib.pyplot as plt
import csv
import sys
import _DataConnector as dc
from io import StringIO
import pickle

#function node has
# 2 Inputs:
#[1] csv file object
#[2] xgboost model parameters
# 4 Outputs:
#[1] model object
#[2] confusion matrix
#[3] error identifier list
#[4] feature importance picture object

#helper functions
###############################################################################################
#make all numerical
def label_encode(df):
    cate = [key for key in dict(df.dtypes) if dict(df.dtypes)[key] in ['bool', 'object']]
    # print(cate)
    le = LabelEncoder()
    for i in cate:
        le.fit(df[i])
        df[i] = le.transform(df[i])
    return df

def sigmoid(x):
    return 1/(1+ np.exp(-x))

# Log loss for logistic regression
def cust_obj(preds, training):
    labels = training.get_label()
    preds = sigmoid(preds)
    grad = preds - labels
    hess = preds*(1-preds)
    return grad, hess
##############################################################################################
#Input1


tableID = pickle.load(open(sys.argv[0], "rb"))
buffer = dc.ReadBuffer(tableID)

telco1 = pd.read_csv(StringIO(buffer))

#data cleanning, remove sensitive feature
df = telco1
op1 = 'phone number'
op2 = 'churn'

df = df.drop([op1],axis=1) # drop phone number

# Separate and rewrite churn outcomes
le = LabelEncoder()
y = df[op2]
df = df.drop([op2],axis=1)
y = le.fit_transform(y)
df = label_encode(df)
X = df.to_numpy()

#test-train split
holdout_X, X, holdout_y, y = train_test_split(X, y, train_size=0.25, random_state=42)

#build and train model
dtest = xgb.DMatrix(np.asarray(X[:int(len(X))]), label=np.asarray(y[:int(len(y))]))

dtrainAll = xgb.DMatrix(np.asarray(holdout_X[:int(len(holdout_X))]), label=np.asarray(holdout_y[:int(len(holdout_y))]))

#Input 2

params = pickle.load(open(sys.argv[1], "rb"))
#{'max_depth': 8, 'n_estimators': 100, 'colsample_bytree': 0.8, 'subsample': 0.8, 'nthread': 10, 'learning_rate': 0.1}

#output1
modelAll = xgb.Booster(params, [dtrainAll])
for i in range(20):
    predictions = modelAll.predict(dtrainAll)
    g,h = cust_obj(predictions, dtrainAll)
    modelAll.boost(dtrainAll,g,h)

pickle.dump(modelAll, open(sys.argv[2], "wb"))

#use model for prediction on test set
test_preds_all = modelAll.predict(dtest)
test_preds_all = sigmoid(test_preds_all)
test_preds_labels_all = np.round(test_preds_all)

#confusion matrix, Output2
all_conf = confusion_matrix(y[:int(len(y))], test_preds_labels_all)
pickle.dump(all_conf, open(sys.argv[3], "wb"))

#erros
tn, fp, fn, tp = all_conf.ravel()
all_errors = (fn+fp)/(tn+fp+fn+tp) 
all_fnr = fn / (tp+fn)
all_fpr = fp / (tn+fp)

preds = test_preds_all
best_preds = np.asarray([np.argmax(line) for line in preds])

Precision = precision_score(dtest.get_label(), best_preds, average='macro')
Recall=recall_score(dtest.get_label(), best_preds, average='macro')
Accuracy=accuracy_score(dtest.get_label(), best_preds)

#Output3
errors=[all_errors, all_fnr, all_fpr, Precision, Recall, Accuracy]
pickle.dump(errors, open(sys.argv[4], "wb"))

#plot feature importance, Output4
fig = xgb.plot_importance(modelAll)
pickle.dump(fig, open(sys.argv[5], "wb"))


