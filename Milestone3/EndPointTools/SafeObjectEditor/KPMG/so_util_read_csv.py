from io import StringIO
import pandas as pd

csv = StringIO(__csv.decode().replace('\r', ''))
__df  = pd.read_csv(csv, sep='\x1f', error_bad_lines=False)
