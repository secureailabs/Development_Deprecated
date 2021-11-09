import datetime as dt
import pandas as pd
import numpy as np
import scipy


agreement_df = __data0
contact_df = __data1
payment_df = __data2

contact_df = contact_df.drop(["ContactID", "CreateReasonGUID", "CancelReasonGUID", "Age"], axis=1)

agreement_df = agreement_df.drop(["ContactGUID", "AgreementID", "ChannelName", "FutureCancelDate", "ContactID"], axis=1)

payment_df = payment_df.drop(["PaymentID", "AgreementtypeGUID", "PaymentMethodName", "PaymentStatusName"], axis=1)

merged_df = payment_df.merge(agreement_df, on="AgreementGUID", how="left").merge(contact_df, on="ContactGUID", how = "left")

merged_df["is_churned"] = False

churned_df = merged_df[merged_df["CancelDate"].isna() == False].groupby("ContactGUID", as_index = False)["CancelDate"].min()

merged_df = merged_df.merge(churned_df, on="ContactGUID", how="left")

# Is the payment associated with an agreement
merged_df["payment_on_agreement"] = np.where(merged_df["AgreementGUID"].isna(), False, True)

# Convert datatypes
merged_df["PaymentDate"] = pd.to_datetime(merged_df["PaymentDate"])
merged_df["Startdate"] = pd.to_datetime(merged_df["Startdate"])
merged_df["CancelDate_y"] = pd.to_datetime(merged_df["CancelDate_y"])
merged_df["ChargedDate"] = pd.to_datetime(merged_df["ChargedDate"])

merged_df["PaidAmount"] = pd.to_numeric(merged_df["PaidAmount"].str.replace(',','.'))

# Churn flag
merged_df['is_churned'] = False
merged_df['is_churned'] = np.where((merged_df['CancelDate_y'].dt.month <= merged_df["PaymentDate"].dt.month) &
                                   (merged_df['CancelDate_y'].dt.year <= merged_df["PaymentDate"].dt.year), True, False)

# Days since start
merged_df["days_since_start"] = (merged_df["PaymentDate"]- merged_df["Startdate"]).dt.days

# Days since last payment
merged_df = merged_df.sort_values(by=[ "ContactGUID", "PaymentDate"])
merged_df["days_since_last_payment"] = merged_df.groupby(["ContactGUID", "AgreementGUID"])["PaymentDate"].diff().dt.days

# Missing payment
merged_df["missing_payment"] = np.where((merged_df["PaymentDate"].isna() == True) &
                                        (merged_df['CancelDate_y'] >= merged_df["ChargedDate"]), True, False)

one_hot_list = ["PaymentMethod", "PaymentStatus", "ChannelD", "AgreementType", "CreateReason"]
merged_df = pd.get_dummies(merged_df, columns = one_hot_list)

agg_dict = 0
if(__flag == 1):
	agg_dict = {
	    'PaidAmount': 'sum',
	    'AgreementGUID' : 'count',
	    'PaymentGUID' : 'count',
	    'CancelDate_y': 'min',
	    'PostalCode': ['min', 'max'],
	    'Donation': 'sum',
	    'Medlemskab': 'sum',
	    'Product': 'sum',
	    'Lottery': 'sum',
	    'payment_on_agreement': 'sum',
	    'days_since_start': ['min', 'max'],
	    'days_since_last_payment' : ['min', 'max'],
	    'missing_payment' : 'sum',
	    'is_churned': 'max'
	}
else:
	agg_dict = {
	    'PaidAmount': 'sum',
	    'AgreementGUID' : 'count',
	    'PaymentGUID' : 'count',
	    'CancelDate_y': 'min',
	    'PostalCode': ['min', 'max'],
	    'Donation': 'sum',
	    'Membership': 'sum',
	    'Product': 'sum',
	    'Lottery': 'sum',
	    'payment_on_agreement': 'sum',
	    'days_since_start': ['min', 'max'],
	    'days_since_last_payment' : ['min', 'max'],
	    'missing_payment' : 'sum',
	    'is_churned': 'max'
	}

for one_hot_col in one_hot_list:
    tmp_dict = {x: 'sum' for x in merged_df.columns if one_hot_col in x}
    agg_dict.update(tmp_dict)
    
#assert len([x for x in merged_df.columns if x not in agg_dict.keys()])==2, "Columns are missing an agg"

# Group and aggregate
final_df = merged_df.set_index("PaymentDate").groupby(["ContactGUID", pd.Grouper(freq="M")]).agg(agg_dict)

final_df = final_df.reset_index()
final_df.columns = ["_".join(x) for x in final_df.columns.ravel()]

# Churned within three months
final_df["is_churned_in3m"] = np.where((final_df['CancelDate_y_min'].dt.year <= final_df["PaymentDate_"].dt.year) &
                                   (final_df['CancelDate_y_min'].dt.month <= final_df["PaymentDate_"].dt.month+3), True, False)

# Cummulated total paid amount
final_df["cumulated_paid_amount"] = final_df.groupby(["ContactGUID_"])["PaidAmount_sum"].cumsum()

ids = final_df.groupby('ContactGUID_', group_keys=False).first().index.tolist()
date_range = pd.date_range(final_df["PaymentDate_"].min().date(),
                           final_df["PaymentDate_"].max().date()-dt.timedelta(days=0),freq='M')

df = pd.DataFrame(index=ids, columns=date_range)
df['temp']=(' '.join(df.columns.astype(str)))
df=df['temp'].str.split(expand=True)

df["Dates"] = df[df.columns].values.tolist()
df = df[["Dates"]]
df = df.explode("Dates").reset_index().rename(columns = {'index':'ContactGUID_', 'Dates':'PaymentDate_'})

df["PaymentDate_"] = pd.to_datetime(df["PaymentDate_"])

final_df = final_df.merge(df, on=["PaymentDate_", "ContactGUID_"],
                          how='outer', indicator=True).reset_index().sort_values(by=["ContactGUID_", "PaymentDate_"])

# Cumulated paid amount in the last three months
final_df["PaidAmount_sum"] = final_df["PaidAmount_sum"].fillna(0) 

final_df["cumulated_amount_3months"] = final_df[["PaidAmount_sum"]].groupby(
    final_df["ContactGUID_"]).apply(lambda g: g.rolling(3).sum())
final_df["cumulated_amount_6months"] = final_df[["PaidAmount_sum"]].groupby(
    final_df["ContactGUID_"]).apply(lambda g: g.rolling(6).sum())

final_df = final_df[final_df["_merge"] == "both"]

a_vc = final_df["is_churned_in3m"].value_counts()
print(a_vc)

# Drop features that are not needed/not explore yet
final_df = final_df.drop(["index", "ContactGUID_", "PaymentDate_", "CancelDate_y_min", "is_churned_max",
                         "PostalCode_max", "PostalCode_min", "Lottery_sum", "_merge"], axis=1)

final_df["is_churned_in3m"] = final_df["is_churned_in3m"].astype(int)

__X = final_df.drop(["is_churned_in3m"], axis=1)
__y = final_df["is_churned_in3m"]

__X['Donation_sum'].astype(float)
#__X['Product_sum'].astype(float)
if(__flag == 1):
	__X['Medlemskab_sum'].astype(float)
else:
	__X['Membership_sum'].astype(float)
__X['Product_sum']=__X['Product_sum'].astype('category')
__X['Product_sum'] = __X['Product_sum'].cat.codes
