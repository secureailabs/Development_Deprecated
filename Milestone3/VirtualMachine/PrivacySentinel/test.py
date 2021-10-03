import PrivacySentinel as ps
import pandas as pd
import numpy as np
import json

df = pd.DataFrame(np.random.randint(0,100,size=(6, 4)), columns=list('ABCD'))
print(df)

ps.PrivacyValidation(df, "jobid")
print(df.shape[0]['hello'])

print("This should print only if the privacy is not violated.")
