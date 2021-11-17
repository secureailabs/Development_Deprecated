from io import StringIO
import pandas as pd

__dataframe  = pd.read_csv(StringIO(__csv.decode()), sep='\x1f',  error_bad_lines=False, lineterminator="\r")