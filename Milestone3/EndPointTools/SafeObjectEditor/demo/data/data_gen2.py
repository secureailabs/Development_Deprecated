import numpy as np

#AGE
AGE1 = np.random.uniform(30,40,100)
AGE1 = np.rint(AGE1)
AGE1 = AGE1.astype(int)

AGE2 = np.random.uniform(40,50,300)
AGE2 = np.rint(AGE2)
AGE2 = AGE2.astype(int)

AGE = np.concatenate((AGE1, AGE2), axis=None)
np.random.shuffle(AGE)
print(AGE)


#condition

condition1 = np.full(400, 'lung cancer')
condition2 = np.full(400, 'liver cancer')
choice = np.random.randint(2, size = 400).astype(bool)
condition = np.where(choice, condition1, condition2)
print(condition)

#drug
drug1 = np.full(400, 'Atezolizumab')
drug2 = np.full(400, 'Ipilimumab')
choice = np.random.randint(2, size = 400).astype(bool)
drug=np.where(choice, drug1, drug2)
print(drug)

d = {'AGE': AGE.tolist(),
     'on drug': drug.tolist(),
     'condition':condition.tolist()}

import pandas as pd

df = pd.DataFrame(data=d)
print(df)

df.to_csv('2.csv')
