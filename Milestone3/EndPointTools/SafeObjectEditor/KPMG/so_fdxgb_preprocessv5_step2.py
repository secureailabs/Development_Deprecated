import pandas as pd


final_df = __data0

train_dates = pd.date_range(start="2000-01-01", end="2020-11-30")
val_dates = pd.date_range(start="2021-02-01", end="2021-02-28")
test_dates = pd.date_range(start="2021-05-01", end="2021-05-30")

final_df['Donation_sum'].astype(float)
final_df['Medlemskab_sum'].astype(float)
final_df['Product_sum'].astype(float)

train_df = final_df.loc[(slice(None), train_dates),:]
val_df = final_df.loc[(slice(None), val_dates),:]
test_df = final_df.loc[(slice(None), test_dates),:]


# In[77]:


print("Train:\t", train_df.shape[0])
print("Val:\t", val_df.shape[0])
print("Test:\t", test_df.shape[0])


# In[78]:


__X_train = train_df.drop(["is_churned_within3m"], axis=1)
__y_train = train_df["is_churned_within3m"]

__X_val = val_df.drop(["is_churned_within3m"], axis=1)
__y_val = val_df["is_churned_within3m"]

__X_trval = pd.concat([__X_train, __X_val])
__y_trval = pd.concat([__y_train, __y_val])

__X_test = test_df.drop(["is_churned_within3m"], axis=1)
__y_test = test_df["is_churned_within3m"]
