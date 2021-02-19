import sys
import pickle
import csv
import pandas as pd
import _DataConnector as dc
from io import StringIO
import xgboost as xgb
import numpy as np
from sklearn.preprocessing import LabelEncoder
from sklearn.metrics import precision_score, recall_score, accuracy_score, confusion_matrix

def label_encode(df):
    cate = [key for key in dict(df.dtypes) if dict(df.dtypes)[key] in ['bool', 'object']]
    # print(cate)
    le = LabelEncoder()
    for i in cate:
        le.fit(df[i])
        df[i] = le.transform(df[i])
    return df


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

dtest = xgb.DMatrix(np.asarray(X[:int(len(X))]), label=np.asarray(y[:int(len(y))]))

model = pickle.load(open(sys.argv[1], "rb"))

test_preds_all = model.predict(dtest)

preds = test_preds_all
best_preds = np.asarray([np.argmax(line) for line in preds])

Precision = precision_score(dtest.get_label(), best_preds, average='macro')
Recall=recall_score(dtest.get_label(), best_preds, average='macro')
Accuracy=accuracy_score(dtest.get_label(), best_preds)

errors = [Precision, Recall, Accuracy]

pickle.dump(errors, open(sys.argv[2], "wb"))
