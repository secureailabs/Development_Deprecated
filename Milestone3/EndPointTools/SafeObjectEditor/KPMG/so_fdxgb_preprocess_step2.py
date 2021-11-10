import pandas as pd


final_df = __data0

train_dates = pd.date_range(start="2010-01-01", end="2021-03-31")
val_dates = pd.date_range(start="2021-03-01", end="2021-03-31")
test_dates = pd.date_range(start="2021-05-01", end="2021-05-30")

final_df['Donation_sum'].astype(float)
final_df['Medlemskab_sum'].astype(float)
final_df['Product_sum'].astype(float)
#final_df['Product_sum']=final_df['Product_sum'].astype('category')
#final_df['Product_sum'] = final_df['Product_sum'].cat.codes

train_df = final_df.loc[(slice(None), train_dates),:]
val_df = final_df.loc[(slice(None), val_dates),:]
test_df = final_df.loc[(slice(None), test_dates),:]

print("Train:\t", train_df.shape[0])
print("Val:\t", val_df.shape[0])
print("Test:\t", test_df.shape[0])

__X_train = train_df.drop(["is_churned_within3m"], axis=1)
__y_train = train_df["is_churned_within3m"]

__X_val = val_df.drop(["is_churned_within3m"], axis=1)
__y_val = val_df["is_churned_within3m"]

#X_train_val = pd.concat([X_train, X_val])
#y_train_val = pd.concat([y_train, y_val])

__X_test = test_df.drop(["is_churned_within3m"], axis=1)
__y_test = test_df["is_churned_within3m"]
