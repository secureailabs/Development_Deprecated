#input 1 __K kfold
#input 2 __X data X
#input 3 __y data y
#output 1 __Xtrain train set X
#output 2 __Xtest test set X
#output 3 __ytrain train set y
#output 4 __ytest test set y

from sklearn.model_selection import KFold

kf = KFold(n_splits = __K)

__trX = []
__teX = []
__try = []
__tey = []

for train_index, test_index in kf.split(__X):
    __trX.append(__X.iloc[train_index])
    __teX.append(__X.iloc[test_index])
    __try.append(__y.iloc[train_index])
    __tey.append(__y.iloc[test_index])

