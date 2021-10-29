import numpy as np

#postcode
postcode = ['02114', '02128', '02139', '02108', '02125']
pc = np.random.choice(postcode, size=100, p = [0.25, 0.25, 0.25, 0.24, 0.01])
#print(pc)

#AGE
age = np.random.randint(20,60,100)
#print(age)

#salary
salary = np.random.randint(70000,180000,100)
#print(salary)

d = {'age':age.tolist(),
     'postcode':pc.tolist(),
     'salary':salary.tolist()}

import pandas as pd
df = pd.DataFrame(data=d)

s_df = df.query("age>20 & age<60 & postcode=='02125'")

df.to_csv('3.csv')
