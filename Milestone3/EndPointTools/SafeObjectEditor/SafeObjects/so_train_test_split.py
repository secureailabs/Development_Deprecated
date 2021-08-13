from sklearn.model_selection import train_test_split

#input1 __testsize
#input2 __randomstate
#input3 __X
#input4 __y
#output1 __X_train
#output2 __X_test
#output3 __y_train
#output4 __y_test

__X_train, __X_test, __y_train, __y_test = train_test_split(__X, __y, test_size=__testsize, random_state=__randomstate)

