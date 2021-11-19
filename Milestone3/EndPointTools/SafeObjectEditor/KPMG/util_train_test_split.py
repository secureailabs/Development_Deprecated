from sklearn.model_selection import train_test_split

__X_train, __X_test, __y_train, __y_test = train_test_split(__X, __y, test_size=__test_size, train_size=__train_size, random_state=__random_state, shuffle=__shuffle, stratify=__stratify)
