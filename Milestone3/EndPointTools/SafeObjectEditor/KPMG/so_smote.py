import imblearn

from imblearn.over_sampling import SMOTE

oversample = SMOTE(sampling_strategy=__sample_size, n_jobs=-1)
__X_sample, __y_sample = oversample.fit_resample(__X, __y)
