from sklearn.preprocessing import LabelEncoder

#input1 __col
#input2 __df
#output __newdf

le = LabelEncoder()
for i in __col:
    __df[i] = le.fit_transform(__df[i])
__newdf = __df
