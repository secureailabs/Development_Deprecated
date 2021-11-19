from io import StringIO
import pandas as pd

__csv = __csv.replace(b'\r', b'')
__dataframe  = pd.read_csv(StringIO(__csv.decode()), sep='\x1f', error_bad_lines=False)