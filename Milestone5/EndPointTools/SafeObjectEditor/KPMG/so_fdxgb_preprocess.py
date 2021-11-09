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


nchurn_reasons = ["395742a5-dc04-e811-8116-5065f38aea11", "992dee8c-8906-e811-8111-5065f38b4641", "59ae8760-1155-e911-a95b-000d3ab6413d",
"715742a5-dc04-e811-8116-5065f38aea11", "705f85e0-7549-e911-a95a-000d3ab6488a", "5b48a520-4ea1-e811-8139-5065f38b4641",
"4b5742a5-dc04-e811-8116-5065f38aea11", "64c7e893-aa23-e911-a955-000d3ab6488a", "8db566eb-a24d-ea11-a812-000d3ab11e51",
"f0b7e130-5677-eb11-a812-000d3ab25083", "f47f6349-d1cb-e811-8141-5065f38b5621", "7d2626f9-d542-e811-8119-5065f38b5621",
"9bd72131-89b7-ea11-a812-000d3aba8599", "27950b8a-4079-e811-8129-5065f38b5621", "6ed69b43-d642-e811-8119-5065f38b5621",
"2565143c-d91d-eb11-a813-000d3aba8599"]

nchurn_contacts = merged_df[merged_df["Cancelreason"].isin(nchurn_reasons)]["ContactGUID"].unique()
merged_df = merged_df[~merged_df["ContactGUID"].isin(nchurn_contacts)]

merged_df["is_churned"] = False

churned_df = merged_df[merged_df["CancelDate"].isna() == False].groupby("ContactGUID", as_index = False)["CancelDate"].min()

merged_df = merged_df.merge(churned_df, on="ContactGUID", how="left")

merged_df["payment_on_agreement"] = np.where(merged_df["AgreementGUID"].isna(), False, True)

merged_df["PaymentDate"] = pd.to_datetime(merged_df["PaymentDate"])
merged_df["Startdate"] = pd.to_datetime(merged_df["Startdate"])
merged_df["CancelDate_y"] = pd.to_datetime(merged_df["CancelDate_y"])
merged_df["ChargedDate"] = pd.to_datetime(merged_df["ChargedDate"])

merged_df["PaidAmount"] = pd.to_numeric(merged_df["PaidAmount"].str.replace(',','.'))

merged_df['is_churned'] = False
merged_df['is_churned'] = np.where((merged_df['CancelDate_y'].dt.month <= merged_df["PaymentDate"].dt.month) &
                                   (merged_df['CancelDate_y'].dt.year <= merged_df["PaymentDate"].dt.year), True, False)

merged_df = merged_df[~merged_df["PaymentGUID"].isin(merged_df[
    merged_df["PaymentDate"] > merged_df["CancelDate_y"]]["PaymentGUID"])]

merged_df["days_since_start"] = (merged_df["PaymentDate"]- merged_df["Startdate"]).dt.days

merged_df = merged_df.sort_values(by=[ "ContactGUID", "PaymentDate"])
merged_df["days_since_last_payment"] = merged_df.groupby(["ContactGUID", "AgreementGUID"])["PaymentDate"].diff().dt.days

merged_df["missing_payment"] = np.where(((merged_df["PaymentStatus"] == 110690003) & (merged_df["PaidAmount"] == 0)) |
                                       ((merged_df["PaymentStatus"] == 110690004) & (merged_df["PaidAmount"] == 0)), True,
                                        False)

grouped = merged_df.groupby("ContactGUID")
result = []
holder = ''

for group_name, group in grouped:
    holding = False
    for row_index, row in group.iterrows():
        if row['missing_payment'] == True:
            holding = True
            holder = row["PaymentDate"]
            result.append(0) 
        elif row["missing_payment"] == False:
            if holding == False:
                result.append(0)
            if holding == True:
                result.append((row['PaymentDate'] - holder).days)
                
merged_df["days_since_missing_payment"] = result

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
	    'days_since_missing_payment' : ['min', 'max'],
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
	    'days_since_missing_payment' : ['min', 'max'],
	    'missing_payment' : 'sum',
	    'is_churned': 'max'
	}

for one_hot_col in one_hot_list:
    tmp_dict = {x: 'sum' for x in merged_df.columns if one_hot_col in x}
    agg_dict.update(tmp_dict)

final_df = merged_df.set_index("PaymentDate").groupby(["ContactGUID", pd.Grouper(freq="M")]).agg(agg_dict)

final_df = final_df.reset_index()
final_df.columns = ["_".join(x) for x in final_df.columns.ravel()]

final_df["is_churned_in3m"] = np.where((final_df['CancelDate_y_min'].dt.year <= final_df["PaymentDate_"].dt.year) &
                                   (final_df['CancelDate_y_min'].dt.month <= final_df["PaymentDate_"].dt.month+3), True, False)

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

final_df["PaidAmount_sum"] = final_df["PaidAmount_sum"].fillna(0) 

final_df["cumulated_amount_3months"] = final_df[["PaidAmount_sum"]].groupby(
    final_df["ContactGUID_"]).apply(lambda g: g.rolling(3).sum())
final_df["cumulated_amount_6months"] = final_df[["PaidAmount_sum"]].groupby(
    final_df["ContactGUID_"]).apply(lambda g: g.rolling(6).sum())

final_df["PaymentGUID_count"] = final_df["PaymentGUID_count"].fillna(0)

final_df["payment_count_last3m"] = final_df[["PaymentGUID_count"]].groupby(
    final_df["ContactGUID_"]).apply(lambda g: g.rolling(3).sum())
final_df["payment_count_last6m"] = final_df[["PaymentGUID_count"]].groupby(
    final_df["ContactGUID_"]).apply(lambda g: g.rolling(6).sum())

final_df["payment_count_total"] = final_df["PaymentGUID_count"].groupby(
    final_df["ContactGUID_"]).cumsum()

final_df = final_df[final_df["_merge"] == "both"]

final_df = final_df.set_index(["ContactGUID_", "PaymentDate_"])

final_df = final_df.drop(["index", "CancelDate_y_min", "is_churned_max",
                         "PostalCode_max", "PostalCode_min", "Lottery_sum", "_merge"], axis=1)

a_vc =final_df["is_churned_in3m"].value_counts() 
print(a_vc)

final_df["is_churned_in3m"] = final_df["is_churned_in3m"].astype(int)


print(final_df["is_churned_in3m"].value_counts())


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

pd.set_option("display.max_columns", None)

print(__X)
print(__y)
