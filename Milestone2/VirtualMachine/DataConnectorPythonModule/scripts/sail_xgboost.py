import pandas as pd
import xgboost as xgb
from sklearn.model_selection import train_test_split
from sklearn import metrics
import _DataConnector as dc
from io import StringIO
import numpy as np

def xgboost(tableID, x, y, paramDict):
	
	buffer = dc.ReadBuffer(tableID)
	df = pd.read_csv(StringIO(buffer))

	df.species = pd.factorize(df.species)[0]
	Y=df.iloc[:, y]
	X=df.iloc[:, x]

	x_train, x_test, y_train, y_test = train_test_split(X,Y, random_state=4)

	dtrain = xgb.DMatrix(x_train, label = y_train)
	dtest = xgb.DMatrix(x_test, label = y_test)

	param = paramDict

	num_round = 20

	bst = xgb.train(param, dtrain, num_round)

	preds = bst.predict(dtest)
	best_preds = np.asarray([np.argmax(line) for line in preds])

	accuracy = metrics.precision_score(y_test, best_preds, average='macro')

	print(accuracy)

#xgboost("../test/iris.csv", [0,1,2,3], 4, {'max_depth':3, 'eta':0.3, 'silent':1, 'objective':'multi:softprob', 'num_class':3})
