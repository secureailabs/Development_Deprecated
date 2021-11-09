#input 1 __K kfold
#input 2 __X data X
#input 3 __y data y
#output 1 __Xtrain train set X
#output 2 __Xtest test set X
#output 3 __ytrain train set y
#output 4 __ytest test set y

from sklearn.model_selection import KFold

__K = 5
kf = KFold(n_splits = __K)

trainX = []
testX = []
trainy = []
testy = []

for train_index, test_index in kf.split(__X):
    trainX.append(__X.iloc[train_index])
    testX.append(__X.iloc[test_index])
    trainy.append(__y.iloc[train_index])
    testy.append(__y.iloc[test_index])

__trainX1 = trainX[0]
__trainX2 = trainX[1]
__trainX3 = trainX[2]
__trainX4 = trainX[3]
__trainX5 = trainX[4]

__trainy1 = trainy[0]
__trainy2 = trainy[1]
__trainy3 = trainy[2]
__trainy4 = trainy[3]
__trainy5 = trainy[4]

__testX1 = testX[0]
__testX2 = testX[1]
__testX3 = testX[2]
__testX4 = testX[3]
__testX5 = testX[4]

__testy1 = testy[0]
__testy2 = testy[1]
__testy3 = testy[2]
__testy4 = testy[3]
__testy5 = testy[4]
