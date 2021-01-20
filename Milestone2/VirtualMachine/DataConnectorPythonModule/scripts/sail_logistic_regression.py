import pandas as pd
from sklearn.linear_model import LogisticRegression
from sklearn.model_selection import train_test_split
from sklearn import metrics
import _DataConnector as dc
from io import StringIO
import sys

def logistic_regression(tableID, x, y):
	buffer = dc.ReadBuffer(tableID)
	df = pd.read_csv(StringIO(buffer))

	df.species = pd.factorize(df.species)[0]
	Y=df.iloc[:, y]
	X=df.iloc[:, x]

	x_train, x_test, y_train, y_test = train_test_split(X,Y, random_state=4)

	model = LogisticRegression(C=1e5)

	model.fit(x_train, y_train)
	y_predict = model.predict(x_test)

	accuracy = metrics.accuracy_score(y_test, y_predict)

	print(accuracy)

#logistic_regression(0, [0,1,2,3], 4)
