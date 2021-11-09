import pandas as pd

__dummies = pd.get_dummies(__data, prefix=__prefix, prefix_sep=__prefix_sep, dummy_na=__dummy_na, columns=__columns, sparse=__sparse, drop_first=__drop_first, dtype=__dtype)
