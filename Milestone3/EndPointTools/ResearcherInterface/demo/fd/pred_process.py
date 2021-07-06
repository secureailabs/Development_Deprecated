import pickle
import _DataConnector as dc
from io import StringIO
import sys
import pandas as pd
from sklearn.preprocessing import LabelEncoder
import numpy as np

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
df = df.drop([op1],axis=1) # drop phone number

df = label_encode(df)
features = list(df.columns) # Use for SHAP demo later
X = df.to_numpy()
#    X, holdout_X, y, holdout_y = train_test_split(X, y, train_size=0.25, random_state=42)

pickle.dump(X, open(sys.argv[1], "wb"))