from sklearn.model_selection import train_test_split

#input1 __testsize
#input2 __randomstate
#input3 __X
#input4 __y
#output1 __Xtrain
#output2 __Xtest
#output3 __ytrain
#output4 __ytest

__Xtrain, __Xtest, __ytrain, __ytest = train_test_split(__X, __y, test_size=__testsize, random_state=__randomstate)

