import imblearn
from imblearn.over_sampling import SMOTE
from collections import Counter


oversample = SMOTE(sampling_strategy=__sample_size, n_jobs=-1)
print(Counter(__y))

__X_sample, __y_sample = oversample.fit_resample(__X, __y)
print(Counter(__y_sample_))