import imblearn
from imblearn.over_sampling import SMOTE
from collections import Counter


oversample = SMOTE(sampling_strategy=__sampleSize, n_jobs=-1)
print(Counter(__y))

__XSample, __ySample = oversample.fit_resample(__X, __y)
print(Counter(__ySample_))