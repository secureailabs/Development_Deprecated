import pickle
import _DataConnector as dc
from io import StringIO
import sys
import pandas as pd
from sklearn.preprocessing import LabelEncoder
from imblearn.over_sampling import SMOTE
import numpy as np
from sklearn.model_selection import train_test_split

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

#party data process
df = telco1
op1 = 'phone number'
op2 = 'readmission'

df = df.drop([op1],axis=1) # drop phone number

# Separate and rewrite churn outcomes
le = LabelEncoder()
y = df[op2]
df = df.drop([op2],axis=1)
y = le.fit_transform(y)

df = label_encode(df)
features = list(df.columns) # Use for SHAP demo later
X = df.to_numpy()
#    X, holdout_X, y, holdout_y = train_test_split(X, y, train_size=0.25, random_state=42)

holdout_X, X, holdout_y, y = train_test_split(X, y, train_size=0.25, random_state=42)

smote = SMOTE(random_state=42)
X, y = smote.fit_resample(X, y)

pickle.dump(X, open(sys.argv[1], "wb"))
pickle.dump(y, open(sys.argv[2], "wb"))
pickle.dump(holdout_X, open(sys.argv[3], "wb"))
pickle.dump(holdout_y, open(sys.argv[4], "wb"))
pickle.dump(df, open(sys.argv[5], "wb"))
