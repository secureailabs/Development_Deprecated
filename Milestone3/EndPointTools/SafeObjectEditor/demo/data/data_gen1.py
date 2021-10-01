import numpy as np
import matplotlib.pyplot as plt

#BMI
shape, scale = 41., 0.6
BMI = np.random.gamma(shape, scale, 300)
BMI = np.around(BMI,1)

#import scipy.special as sps  
#count, bins, ignored = plt.hist(s, 50, density=True)
#y = bins**(shape-1)*(np.exp(-bins/scale) /  
#                     (sps.gamma(shape)*scale**shape))
#plt.plot(bins, y, linewidth=2, color='r')  
#plt.show()

#Age
shape, scale = 30., 1.7
AGE = np.random.gamma(shape, scale, 300)
AGE = np.trunc(AGE)
AGE = AGE.astype(int)
#print(AGE)
#print(np.amin(AGE))
#print(np.amax(AGE))

#import scipy.special as sps  
#count, bins, ignored = plt.hist(AGE, 50, density=True)
#y = bins**(shape-1)*(np.exp(-bins/scale) /  
#                     (sps.gamma(shape)*scale**shape))
#plt.plot(bins, y, linewidth=2, color='r')  
#plt.show()

#PD-L1 before
PDL1_before = np.random.uniform(80,90,300)
PDL1_before = np.around(PDL1_before,2)
#print(s)

#PD-L1 after
PDL1_after = np.random.uniform(20,40,300)
PDL1_after = np.around(PDL1_after,2)

#PD-L2 before
PDL2_before = np.random.uniform(30,40,300)
PDL2_before = np.around(PDL2_before,2)

#PD-L2 after
PDL2_after = np.random.uniform(25,40,300)
PDL2_after = np.around(PDL2_after,2)

#PD1 before
PD1_before = np.random.uniform(10,30,300)
PD1_before = np.around(PD1_before,2)

#PD1 after
PD1_after = np.random.uniform(8,32,300)
PD1_after = np.around(PD1_after,2)

import pandas as pd

d = {'AGE': AGE.tolist(), 
     'BMI': BMI.tolist(), 
     'PD-L1 level before treatment': PDL1_before.tolist(),
     'PD-L1 level after treatment': PDL1_after.tolist(),
     'PD-L2 level before treatment': PDL2_before.tolist(),
     'PD-L2 level after treatment': PDL2_after.tolist(),
     'PD1 level before treatment': PD1_before.tolist(),
     'PD1 level after treatment': PD1_after.tolist()}

df = pd.DataFrame(data = d)
print(df)
diff = df['PD-L1 level before treatment'].mean() - df['PD-L1 level after treatment'].mean()
print(diff)

df.to_csv('1.csv')




